//
// Created by gabriel on 25/01/17.
//

#include "CIP_Connection.h"

CipStatus CIP_Connection::Init()
{
    class_id = 5;
    class_name = "Connection";
    class_id = kCipConnectionManagerClassCode;
    get_all_class_attributes_mask = 0xC6;
    get_all_instance_attributes_mask = 0xffffffff;
    class_name = "Connection Manager";
    revision = 1;
    revision = 0;

    CIP_Connection *instance = new CIP_Connection();
    object_Set.emplace(object_Set.size(), instance);

    //Chapter 4 vol 1
    instance->InsertAttribute(1, , , , );
    instance->InsertAttribute(2, , , , );
    instance->InsertAttribute(3, , , , );
    instance->InsertAttribute(4, , , , );
    instance->InsertAttribute(5, , , , );
    instance->InsertAttribute(6, , , , );
    instance->InsertAttribute(7, , , , );
    //Chapter 5 vol 5
    instance->InsertAttribute(8, , , , );
    instance->InsertAttribute(9, , , , );

    //Class services
}



CipStatus CIP_Connection::InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response)
{
    //Class services
    if (this->id == 0)
    {
        switch (service)
        {
            case (kConnectionServiceCreate):
                break;
            case (kConnectionServiceDelete):
                break;
            case (kConnectionServiceReset):
                break;
            case (kConnectionServiceFindNextInstance):
                break;
            case (kConnectionServiceGetAttributeSingle):
                break;
            default:
                return kCipStatusError;
        }
        return kCipStatusOk;
    }
    //Instance services
    else
    {
        switch(service)
        {
            case (kConnectionInstServiceBind):
                break;
            case (kConnectionInstServiceProducingLookup):
                break;
            case (kConnectionInstServiceSafetyClose):
                break;
            case (kConnectionInstServiceSafetyOpen):
                break;
            default:
                return kCipStatusError;
        }
        return kCipStatusOk;
    }
}