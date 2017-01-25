//
// Created by gabriel on 25/01/17.
//

#include <typedefs.hpp>
#include <vector>
#include <cstring>
#include "CIP_Connection.hpp"

CipStatus CIP_Connection::Init()
{
    class_id = 5;
    class_name = "Connection";
    class_id = kCipConnectionManagerClassCode;
    class_name = "Connection Manager";
    revision = 1;

    CIP_Connection *instance = new CIP_Connection();

    //Chapter 4 vol 1 - Common CIP attributes added on template itself
    instance->InsertAttribute(1, kCipUint , &revision                             , kGetableSingleAndAll);
    instance->InsertAttribute(2, kCipUint , &max_instances                        , kGetableSingleAndAll);
    instance->InsertAttribute(3, kCipUint , &number_of_instances                  , kGetableSingleAndAll);
    instance->InsertAttribute(4, kCipUdint, &optional_attribute_list              , kGetableSingleAndAll);
    instance->InsertAttribute(5, kCipUdint, &optional_service_list                , kGetableSingleAndAll);
    instance->InsertAttribute(6, kCipUint , &maximum_id_number_class_attributes   , kGetableSingleAndAll);
    instance->InsertAttribute(7, kCipUint , &maximum_id_number_instance_attributes, kGetableSingleAndAll);

    //Chapter 5 vol 5
    //todo: recheck sizes
    instance->InsertAttribute(8, kCipUint, &ConnectionRequestErrorCount, kGetableSingleAndAll));
    instance->InsertAttribute(9, kCipUint, &SafetyConnectionCounters   , kGetableSingleAndAll));


    object_Set.emplace(object_Set.size(), instance);

    //Class services
}

//Class services
CipStatus CIP_Connection::Create()
{
    CIP_Connection *instance = new CIP_Connection();
    //Chapter 3-4.4 vol 1
    instance->InsertAttribute( 1, kCipUsint, &State                                , kGetableSingleAndAll);
    instance->InsertAttribute( 2, kCipUsint, &InstanceT_type                       , kGetableSingleAndAll);
    instance->InsertAttribute( 3, kCipByte , &TransportClass_trigger               , kGetableSingleAndAll);
    instance->InsertAttribute( 4, kCipUint , &DeviceNet_produced_connection_id     , kGetableSingleAndAll);
    instance->InsertAttribute( 5, kCipUint , &DeviceNet_consumed_connection_id     , kGetableSingleAndAll);
    instance->InsertAttribute( 6, kCipByte , &DeviceNet_initial_comm_characteristcs, kGetableSingleAndAll);
    instance->InsertAttribute( 7, kCipUint , &Produced_connection_size             , kGetableSingleAndAll);
    instance->InsertAttribute( 8, kCipUint , &Consumed_connection_size             , kGetableSingleAndAll);
    instance->InsertAttribute( 9, kCipUint , &Expected_packet_rate                 , kGetableSingleAndAll);
    instance->InsertAttribute(10, kCipUdint, &CIP_produced_connection_id           , kGetableSingleAndAll);
    instance->InsertAttribute(11, kCipUdint, &CIP_consumed_connection_id           , kGetableSingleAndAll);
    instance->InsertAttribute(12, kCipUsint, &Watchdog_timeout_action              , kGetableSingleAndAll);
    instance->InsertAttribute(13, kCipUint , &Produced_connection_path_length      , kGetableSingleAndAll);
    instance->InsertAttribute(14, kCipEpath, &Produced_connection_path             , kGetableSingleAndAll);
    instance->InsertAttribute(15, kCipUint , &Consumed_connection_path_length      , kGetableSingleAndAll);
    instance->InsertAttribute(16, kCipEpath, &Consumed_connection_path             , kGetableSingleAndAll);
    instance->InsertAttribute(17, kCipUint , &Production_inhibit_time              , kGetableSingleAndAll);
    instance->InsertAttribute(18, kCipUsint, &Connection_timeout_multiplier        , kGetableSingleAndAll);
    instance->InsertAttribute(19, kCipUdint, &Connection_binding_list              , kGetableSingleAndAll);
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
CipStatus CIP_Connection::Bind(CipUint bound_instances[2])
{
    CipStatus status;
    if (GetInstance(bound_instances[0]) == nullptr & GetInstance(bound_instances[1]) == nullptr)
    {
        //if both connections exist, then
        //check if there are resources to bound
        if ()
        {
            if (GetInstance(bound_instances[0])->State == kConnectionStateEstablished & GetInstance(bound_instances[1])->State == kConnectionStateEstablished)
            {
                if (bound_instances[0] != bound_instances[1])
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

CipStatus CIP_Connection::ProducingLookup(CipEpath producing_application_path, CipUint *instance_count, std::vector<CipUint> connection_instance_list[])
{
    int j;
    CipStatus status;
    if ( ( j = object_Set.size() ) < 1)
    {
        status.status = 0x02;
        status.extended_status = 0x01;
        return status;
    }
    const CIP_Connection * obj_ptr;

    //Check every connection to check out if active and producing
    CipUint k;
    for (unsigned int i = 0; i < j; i++)
    {
        obj_ptr = GetInstance(i);
        if (obj_ptr->is_active & obj_ptr->is_producing)
        {
            if (strcmp(producing_application_path, obj_ptr->Produced_connection_path) == 0)
            {
                (*instance_count)++;
                connection_instance_list->push_back(k = obj_ptr->id);
            }
        }
    }



    status.status = 0x0;
    status.extended_status = 0x0;
    return status;
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