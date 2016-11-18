#include "CIP_ClassInstance.h"
#include "trace.h"
#include "src/cip/connection_stack/cipcommon.h"
#include "CIP_Service.h"
#include "CIP_Attribute.h"
#include <utility>
#include "../CIP_Protocol.h"


/*CIP_ClassInstance::CIP_ClassInstance(CipUdint class_id, CipUdint get_all_class_attributes_mask, CipUdint get_all_instance_attributes_mask,
                     CipUdint position, CipUint revision)
{

    OPENER_TRACE_INFO("creating class instance '%s' with id: 0x%" PRIX32 "\n", name,  class_id);

    // initialize the class-specific fields of the Class struct

    // the class remembers the class ID 
    this->class_id = class_id;

    // the class remembers the class ID 
    this->revision = revision;

    // indicate which attributes are included in instance getAttributeAll  
    this->get_all_instance_attributes_mask = get_all_instance_attributes_mask;
    this->get_all_class_attributes_mask = get_all_class_attributes_mask;

    // create the standard class attributes
    if (CIP_Protocol::GetCipClass(class_id) == NULL)
    {
        // revision
        this->InsertAttribute(1, kCipUint, (void *) &this->revision, kGetableSingleAndAll);

        //  largest instance number
        this->InsertAttribute(2, kCipUint, NULL, kGetableSingleAndAll);

        // number of instances currently existing
        this->InsertAttribute(3, kCipUint, NULL, kGetableSingleAndAll);

        // optional attribute list - default = 0
        this->InsertAttribute(4, kCipUint, (void *) &kCipUintZero, kGetableAll);

        // optional service list - default = 0
        this->InsertAttribute(5, kCipUint, (void *) &kCipUintZero, kGetableAll);

        // max class attribute number
        this->InsertAttribute(6, kCipUint, NULL, kGetableSingleAndAll);

        // max instance attribute number
        this->InsertAttribute(7, kCipUint, NULL, kGetableSingleAndAll);

        // create the standard instance services
        // only if the mask has values add the get_attribute_all service
        if (0 != get_all_instance_attributes_mask) {
            // bind instance services to the class
            this->InsertService(kGetAttributeAll, &this->GetAttributeAll, "GetAttributeAll");
        }
        this->InsertService(kGetAttributeSingle, &this->GetAttributeSingle, "GetAttributeSingle");
        CIP_Protocol::AddCipClassInstance(this, 0);
    }
}*/

CIP_ClassInstance::CIP_ClassInstance()
{

}

CIP_ClassInstance::~CIP_ClassInstance()
{

}

void CIP_ClassInstance::InsertAttribute(CipUint attribute_number, CipUsint cip_type, void * data, CipAttributeFlag cip_flags)
{
    auto it = this->attributes.find(attribute_number);

    /* cant add attribute that already exists */
    if (it != this->attributes.end())
    {
        OPENER_ASSERT(true);
    }
    else
    {
        CIP_Attribute* attribute_ptr = new CIP_Attribute(attribute_number, cip_type, data, cip_flags);

        this->attributes.emplace(attribute_number, attribute_ptr);
        return;
        
    }
    OPENER_ASSERT(false);
    /* trying to insert too many attributes*/
}

void CIP_ClassInstance::InsertService(CipUsint service_number, CipServiceFunction service_function, std::string service_name)
{
    auto it = this->services.find(service_number);

    /* cant add service that already exists */
    if (it == std::map::end())
    {
        OPENER_ASSERT(true);
    }
    else
    {
        CIP_Service* p = new CIP_Service(service_number, service_function, service_name);
        this->services.emplace(service_number, p);

        return;
    }
    OPENER_ASSERT(0);
    /* adding more services than were declared is a no-no*/
}

CIP_Attribute* CIP_ClassInstance::GetCipAttribute(CipUint attribute_number)
{
    auto it = this->attributes[attribute_number]; /* init pointer to array of attributes*/
    //TODO: Check that again
    if (it == std::end)
    {
        OPENER_TRACE_WARN("attribute %d not defined\n", attribute_number);

        return 0;  
    }
    else
    {
        return it;
    }

    
}

/* TODO: this needs to check for buffer overflow*/
CipStatus CIP_ClassInstance::GetAttributeSingle(CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    // Mask for filtering get-ability
    CipByte get_mask;

    CIP_Attribute* attribute = this->GetCipAttribute(message_router_request->request_path.attribute_number);
    CipByte* message = message_router_response->data;

    message_router_response->data_length = 0;
    message_router_response->reply_service = (0x80 | message_router_request->service);
    message_router_response->general_status = kCipErrorAttributeNotSupported;
    message_router_response->size_of_additional_status = 0;

    // set filter according to service: get_attribute_all or get_attribute_single
    if (kGetAttributeAll == message_router_request->service)
    {
        get_mask = kGetableAll;
        message_router_response->general_status = kCipErrorSuccess;
    }
    else
    {
        get_mask = kGetableSingle;
    }

    if ((attribute != 0) && (attribute->getData() != 0))
    {
        if (attribute->getFlag() & get_mask)
        {
            OPENER_TRACE_INFO("getAttribute %d\n",
                message_router_request->request_path.attribute_number); /* create a reply message containing the data*/
            /*TODO think if it is better to put this code in an own
               * getAssemblyAttributeSingle functions which will call get attribute
               * single.
               */

            if (attribute->getType() == kCipByteArray && this->class_id == kCipAssemblyClassCode)
            {
                // we are getting a byte array of a assembly object, kick out to the app callback
                OPENER_TRACE_INFO(" -> getAttributeSingle CIP_BYTE_ARRAY\r\n");
                BeforeAssemblyDataSend(this);
            }

            message_router_response->data_length = EncodeData(attribute->getType(), attribute->getData(), &message);
            message_router_response->general_status = kCipErrorSuccess;
        }
    }

    return kCipStatusOkSend;
}

CipStatus CIP_ClassInstance::GetAttributeAll(CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    int i, j;
    CipUsint* reply;
    CIP_ClassInstance * class_ptr = GetCipClass(this->class_id);

    // pointer into the reply
    reply = message_router_response->data;

    if (GetCipInstanceNumber(this) == 2) {
        OPENER_TRACE_INFO("GetAttributeAll: instance number 2\n");
    }

    CIP_Service * service;
    CIP_Attribute* attribute;
    for (i = 0; i < this->services.size(); i++) /* hunt for the GET_ATTRIBUTE_SINGLE service*/
    {
        // found the service
        if (this->services[i]->getNumber () == kGetAttributeSingle)
        {
            service = this->services[i];
            if (0 == this->attributes.size())
            {
                //there are no attributes to be sent back
                message_router_response->data_length = 0;
                message_router_response->reply_service = (0x80 | message_router_request->service);
                message_router_response->general_status = kCipErrorServiceNotSupported;
                message_router_response->size_of_additional_status = 0;
            }
            else
            {
                for (j = 0; j < class_ptr->attributes.size(); j++) /* for each instance attribute of this class */
                {
                    attribute = attributes[j];
                    int attrNum = attribute->getNumber();

                    // only return attributes that are flagged as being part of GetAttributeALl
                    if (attrNum < 32 && (class_ptr->get_all_class_attributes_mask & 1 << attrNum))
                    {
                        message_router_request->request_path.attribute_number = attrNum;
                        if (kCipStatusOkSend != service->getService()(this, message_router_request, message_router_response))
                        {
                            message_router_response->data = reply;
                            return kCipStatusError;
                        }
                        message_router_response->data += message_router_response->data_length;
                    }
                }
                message_router_response->data_length = message_router_response->data - reply;
                message_router_response->data = reply;
            }
            return kCipStatusOkSend;
        }
        service++;
    }
    return kCipStatusOk; /* Return kCipStatusOk if cannot find GET_ATTRIBUTE_SINGLE service*/
}

CIP_ClassInstance * CIP_ClassInstance::GetCipClassInstance(CipUdint class_id, CipUdint instance_number)
{
    if (CIP_object_set[class_id].size() >= instance_number)
        return CIP_object_set[class_id][instance_number];
    else
        return NULL;
}

CIP_ClassInstance * CIP_ClassInstance::GetCipClass(CipUdint class_id)
{
    if (CIP_object_set[class_id].size() > 0)
        return CIP_object_set[class_id][0];
    else
        return NULL;
}

CipUdint CIP_ClassInstance::GetCipClassNumberInstances(CipUdint class_id)
{
    return CIP_object_set[class_id].size();
}

CipUdint CIP_ClassInstance::GetCipInstanceNumber(CIP_ClassInstance * instance)
{
    return std::distance(CIP_object_set[instance->class_id].begin(), CIP_object_set[instance->class_id].find(instance) );
}

bool CIP_ClassInstance::AddCipClassInstance(CIP_ClassInstance* instance, CipUdint position)
{
    CIP_object_set[instance->class_id].emplace(position,instance);
    auto it = CIP_object_set[instance->class_id].find(position);
    return (it != CIP_object_set[instance->class_id].end());
}