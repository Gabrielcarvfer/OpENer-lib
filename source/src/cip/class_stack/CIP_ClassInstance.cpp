#include "../../trace.h"
#include "CIP_ClassInstance.h"
#include "../connection_stack/CIP_Common.h"
#include "CIP_Attribute.h"
#include "../../opener_user_conf.h"
#include <utility>

//Methods
template <typename T>
CIP_ClassInstance<T>::CIP_ClassInstance()
{
    id = instancesNum;
    instancesNum++;

    if (id == 0)
    {
        class_ptr = this;
    }
}

template <typename T>
CIP_ClassInstance<T>::~CIP_ClassInstance()
{
    instancesNum--;
}

template <typename T>
void CIP_ClassInstance<T>::InsertAttribute(CipUint attribute_number, CipUsint cip_type, void * data, CipAttributeFlag cip_flags)
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

/*template <typename T>
void CIP_ClassInstance::InsertService(CipUsint service_number, CipServiceFunction * service_function, std::string service_name)
{
    auto it = this->services.find(service_number);

    // cant add service that already exists
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
    // adding more services than were declared is a no-no
}*/

template <typename T>
CIP_Attribute* CIP_ClassInstance<T>::GetCipAttribute(CipUint attribute_number)
{
    if (this->attributes.find(attribute_number) == this->attributes.end ())
    {
        OPENER_TRACE_WARN("attribute %d not defined\n", attribute_number);

        return 0;  
    }
    else
    {
        return this->attributes[attribute_number];
    }

    
}

/* TODO: this needs to check for buffer overflow*/
template <typename T>
CipStatus CIP_ClassInstance<T>::GetAttributeSingle(CipMessageRouterRequest* message_router_request,
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

                //TODO:build an alternative
                //BeforeAssemblyDataSend(this);
            }

            message_router_response->data_length = CIP_Common::EncodeData(attribute->getType(), attribute->getData(), &message);
            message_router_response->general_status = kCipErrorSuccess;
        }
    }

    return kCipStatusOkSend;
}

template <typename T>CipStatus CIP_ClassInstance<T>::GetAttributeAll(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{
    int i, j;
    CipUsint* reply;

    // pointer into the reply
    reply = message_router_response->data;

    if (this->id == 2)
    {
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
                        if (kCipStatusOkSend != this->InstanceServices(kGetAttributeAll, message_router_request, message_router_response))
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

