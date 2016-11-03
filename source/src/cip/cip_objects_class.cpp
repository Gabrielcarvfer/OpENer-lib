#include "cip_objects_class.h"

CIPClass::CIPClass( 
                    CipUdint class_id, 
                    int number_of_class_attributes,
                    CipUdint get_all_class_attributes_mask,
                    int number_of_class_services,
                    int number_of_instance_attributes,
                    CipUdint get_all_instance_attributes_mask,
                    int number_of_instance_services,
                    int number_of_instances, 
                    std::string name,
                    CipUint revision
                    ) 
{

    OPENER_TRACE_INFO("creating class instance '%s' with id: 0x%" PRIX32 "\n", name,
        class_id);

    /* initialize the class-specific fields of the Class struct*/

    // the class remembers the class ID 
    class_id = class_id; 

    // the class remembers the class ID 
    revision = revision; 

    // the number of instances initially zero (more created below) 
    number_of_instances = 0; 

    // the class remembers the number of instances of that class 
    number_of_attributes = number_of_instance_attributes;

    // indicate which attributes are included in instance getAttributeAll  
    get_attribute_all_mask = get_all_instance_attributes_mask; 

    // the class manages the behavior of the instances 
    number_of_services = number_of_instance_services + ((0 == get_all_instance_attributes_mask) ? 1 : 2); 

    /* initialize the class-specific fields of the metaClass struct */
    class_name = std::string(name); 

    highest_attribute_number = 0;

    //if ((RegisterCIPClass(class)) == kCipStatusError) { /* no memory to register class in Message Router */
    //    return 0; /*TODO handle return value and clean up if necessary*/
    //}

    /* create the standard class attributes*/
    // revision 
    this.InsertAttribute(1, kCipUint, (void *) &this.revision, kGetableSingleAndAll); 

    //  largest instance number 
    this.InsertAttribute(2, kCipUint, (void *) &this.number_of_instances, kGetableSingleAndAll);     

    // number of instances currently existing
    this.InsertAttribute(3, kCipUint, (void *) &this.number_of_instances, kGetableSingleAndAll); 
    
    // optional attribute list - default = 0 
    this.InsertAttribute(4, kCipUint, (void *) &kCipUintZero, kGetableAll); 

    // optional service list - default = 0 
    this.InsertAttribute(5, kCipUint, (void *) &kCipUintZero, kGetableAll); 

    // max class attribute number
    this.InsertAttribute(6, kCipUint, (void *) &this.highest_attribute_number, kGetableSingleAndAll); 
    
    // max instance attribute number
    this.InsertAttribute(7, kCipUint, (void *) &this.highest_attribute_number, kGetableSingleAndAll); 
    
    /* create the standard instance services*/
    // only if the mask has values add the get_attribute_all service 
    if (0 != get_all_instance_attributes_mask) 
    {
        // bind instance services to the class
        this.InsertService(class, kGetAttributeAll, &this.GetAttributeAll, "GetAttributeAll"); 
    }
    this.InsertService(class, kGetAttributeSingle, &this.GetAttributeSingle, "GetAttributeSingle");

    //Insert to set
    CIP_object_set[class_id].insert(CIP_object_set[class_id].size(), this);
    return this;
}

CIPClass::~CIPClass()
{
    CIP_object_set[this.class_id].erase(this);
    delete services;
    delete attributes;
    delete this;
}

void * CIPClass::GetCipInstance(CipUdint instance_number)
{
    auto it = CIPClass::CIP_object_set[this.class_id].find([instance_number]);

    if (it != std::end)
        /* if the number matches, return the instance*/
        return &CIPClass::CIP_object_set[this.class_id][instance_number]; 

    return NULL;
}

void CIPClass::InsertAttribute(CipUint attribute_number, CipUsint cip_type, void* data, CipByte cip_flags)
{
    auto it = this.attributes.find(attribute_number);

    /* cant add attribute that already exists */
    if (it == std::end)
    {
        OPENER_ASSERT(true);
    }
    else
    {
        CipAttributeStruct* attribute = new CipAttributeStruct();

        attribute->attribute_number = attribute_number;
        attribute->type = cip_type;
        attribute->attribute_flags = cip_flags;
        attribute->data = data;

        /* remember the max attribute number that was defined*/
        if (attribute_number > this.highest_attribute_number) 
        {
            this.highest_attribute_number = attribute_number;
        }

        this.attributes.insert(attribute_number, attribute);

        return;
        
    }
    OPENER_ASSERT(false);
    /* trying to insert too many attributes*/
}

void CIPClass::InsertService(CipUsint service_number, CipServiceFunction service_function, char* service_name)
{
    auto it = this.attributes.find(attribute_number);

    /* cant add attribute that already exists */
    if (it == std::end)
    {
        OPENER_ASSERT(true);
    }
    else
    {
        CipServiceStruct* p = new CipServiceStruct();
        
        /* fill in service number*/
        p->service_number = service_number; 

        /* fill in function address*/
        p->service_function = service_function; 
        p->name = service_name;
        this.services.insert(service_number, p);

        return;
    }
    OPENER_ASSERT(0);
    /* adding more services than were declared is a no-no*/
}

CipAttributeStruct* CIPClass::GetCipAttribute(CipUint attribute_number)
{
    auto it = this.attributes[attribute_number]; /* init pointer to array of attributes*/
    //TODO: Check that again
    if (it == std::end)
    {
        OPENER_TRACE_WARN("attribute %d not defined\n", attribute_number);

        return 0;  
    }
    else
    {
        return &it;
    }

    
}

/* TODO: this needs to check for buffer overflow*/
CipStatus CIPSharedUtils::GetAttributeSingle(CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    /* Mask for filtering get-ability */
    CipByte get_mask;

    CipAttributeStruct* attribute = this.GetCipAttribute(message_router_request->request_path.attribute_number);
    CipByte* message = message_router_response->data;

    message_router_response->data_length = 0;
    message_router_response->reply_service = (0x80
        | message_router_request->service);
    message_router_response->general_status = kCipErrorAttributeNotSupported;
    message_router_response->size_of_additional_status = 0;

    /* set filter according to service: get_attribute_all or get_attribute_single */
    if (kGetAttributeAll == message_router_request->service) {
        get_mask = kGetableAll;
        message_router_response->general_status = kCipErrorSuccess;
    } else {
        get_mask = kGetableSingle;
    }

    if ((attribute != 0) && (attribute->data != 0)) {
        if (attribute->attribute_flags & get_mask) {
            OPENER_TRACE_INFO("getAttribute %d\n",
                message_router_request->request_path.attribute_number); /* create a reply message containing the data*/

            /*TODO think if it is better to put this code in an own
       * getAssemblyAttributeSingle functions which will call get attribute
       * single.
       */

            if (attribute->type == kCipByteArray
                && CIPClass.CIP_class_set[class_id].class_id == kCipAssemblyClassCode) {
                /* we are getting a byte array of a assembly object, kick out to the app callback */
                OPENER_TRACE_INFO(" -> getAttributeSingle CIP_BYTE_ARRAY\r\n");
                BeforeAssemblyDataSend(instance);
            }

            message_router_response->data_length = EncodeData(attribute->type,
                attribute->data,
                &message);
            message_router_response->general_status = kCipErrorSuccess;
        }
    }

    return kCipStatusOkSend;
}

CipStatus CIPSharedUtils::GetAttributeAll(CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    int i, j;
    CipUsint* reply;
    CipAttributeStruct* attribute;
    CipServiceStruct* service;

    reply = message_router_response->data; /* pointer into the reply */
    attribute = this.attributes; /* pointer to list of attributes*/
    service = this.services; /* pointer to list of services*/

    if (this->instance_number == 2) {
        OPENER_TRACE_INFO("GetAttributeAll: instance number 2\n");
    }

    for (i = 0; i < instance->cip_class->number_of_services; i++) /* hunt for the GET_ATTRIBUTE_SINGLE service*/
    {
        if (service->service_number == kGetAttributeSingle) /* found the service */
        {
            if (0 == instance->cip_class->number_of_attributes) {
                message_router_response->data_length = 0; /*there are no attributes to be sent back*/
                message_router_response->reply_service = (0x80
                    | message_router_request->service);
                message_router_response->general_status = kCipErrorServiceNotSupported;
                message_router_response->size_of_additional_status = 0;
            } else {
                for (j = 0; j < instance->cip_class->number_of_attributes; j++) /* for each instance attribute of this class */
                {
                    int attrNum = attribute->attribute_number;
                    if (attrNum < 32
                        && (instance->cip_class->get_attribute_all_mask & 1 << attrNum)) /* only return attributes that are flagged as being part of GetAttributeALl */
                    {
                        message_router_request->request_path.attribute_number = attrNum;
                        if (kCipStatusOkSend
                            != service->service_function(instance, message_router_request,
                                   message_router_response)) {
                            message_router_response->data = reply;
                            return kCipStatusError;
                        }
                        message_router_response->data += message_router_response
                                                             ->data_length;
                    }
                    attribute++;
                }
                message_router_response->data_length = message_router_response->data
                    - reply;
                message_router_response->data = reply;
            }
            return kCipStatusOkSend;
        }
        service++;
    }
    return kCipStatusOk; /* Return kCipStatusOk if cannot find GET_ATTRIBUTE_SINGLE service*/
}
