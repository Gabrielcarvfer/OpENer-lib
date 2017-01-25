//
// Created by gabriel on 25/01/17.
//

#include <typedefs.hpp>
#include "CIP_Connection.hpp"

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

//Class services
CipStatus CIP_Connection::Create()
{

}

CipStatus CIP_Connection::Delete()
{

}

CipStatus CIP_Connection::Reset()
{

}

CipStatus CIP_Connection::FindNextInstance()
{

}

CipStatus CIP_Connection::GetAttributeSingle()
{

}

//Instance services
CipStatus CIP_Connection::Bind(ConnectionHandles connHandle)
{
    CipStatus status;
    if (GetInstance(connHandle.handle[0]) == nullptr & GetInstance(connHandle.handle[1]) == nullptr)
    {
        //if both connections exist, then
        //check if there are resources to bound
        if ()
        {
            if (GetInstance(connHandle.handle[0])->state == kConnectionStateEstablished & GetInstance(connHandle.handle[1])->state == kConnectionStateEstablished)
            {
                if (connHandle.handle[0] != connHandle.handle[1])
                {
                    //check if one or both instances are not dynamically created I/O conn
                    if ()
                    {
                        //check if connections are ccreated internaly and device prevent bind
                        if ()
                        {
                            status.status=kCipStatusOk;
                            return status;
                        }
                        //Device prevent binding
                        status.extended_status = 0x02;
                        status.status = 0xD0;
                        return status;
                    }
                    //At least one connection is not dynamically created
                    status.extended_status = 0x01;
                    status.status = 0xD0;
                    return status;
                }
                //Both connections are the same
                status.extended_status = 0x01;
                status.status = kCipErrorInvalidParameter;
                return status;
            }
            //Both instances exist, but at least one is not in Established state
            status.extended_status = 0x01;
            status.status = kCipErrorObjectStateConflict;
            return status;
        }
        //Class or instance out of resources to bind
        status.extended_status = 0x02;
        status.status = kCipErrorResourceUnavailable;
        return status;
    }
    //One or both connections dont exist
    status.extended_status = 0x01;
    status.status = kCipErrorResourceUnavailable;
    return status;
}

CipStatus CIP_Connection::ProducingLookup()
{

}

CipStatus CIP_Connection::SafetyClose()
{

}

CipStatus CIP_Connection::SafetyOpen()
{

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