//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#include <typedefs.hpp>
#include <vector>
#include <cstring>
#include <cip/CIP_Objects/CIP_0002_MessageRouter/CIP_MessageRouter.hpp>
#include "CIP_Connection.hpp"

CipStatus CIP_Connection::Init()
{
    if (number_of_instances == 0)
    {
        class_id = 5;
        class_name = "Connection";
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
        //instance->InsertAttribute(8, kCipUint, &ConnectionRequestErrorCount, kGetableSingleAndAll));
        //instance->InsertAttribute(9, kCipUint, &SafetyConnectionCounters   , kGetableSingleAndAll));


        object_Set.emplace(object_Set.size(), instance);

        //Class services
    }
    return kCipStatusOk;
}

//Class services
CipStatus CIP_Connection::Create()
{
    CIP_Connection *instance = new CIP_Connection();
    //Chapter 3-4.4 vol 1
    instance->InsertAttribute( 1, kCipUsint, &instance->State                                 , kGetableSingleAndAll);
    instance->InsertAttribute( 2, kCipUsint, &instance->Instance_type                         , kGetableSingleAndAll);
    instance->InsertAttribute( 3, kCipByte , &instance->TransportClass_trigger                , kGetableSingleAndAll);
    instance->InsertAttribute( 4, kCipUint , &instance->DeviceNet_produced_connection_id      , kGetableSingleAndAll);
    instance->InsertAttribute( 5, kCipUint , &instance->DeviceNet_consumed_connection_id      , kGetableSingleAndAll);
    instance->InsertAttribute( 6, kCipByte , &instance->DeviceNet_initial_comm_characteristics, kGetableSingleAndAll);
    instance->InsertAttribute( 7, kCipUint , &instance->Produced_connection_size              , kGetableSingleAndAll);
    instance->InsertAttribute( 8, kCipUint , &instance->Consumed_connection_size              , kGetableSingleAndAll);
    instance->InsertAttribute( 9, kCipUint , &instance->Expected_packet_rate                  , kGetableSingleAndAll);
    instance->InsertAttribute(10, kCipUdint, &instance->CIP_produced_connection_id            , kGetableSingleAndAll);
    instance->InsertAttribute(11, kCipUdint, &instance->CIP_consumed_connection_id            , kGetableSingleAndAll);
    instance->InsertAttribute(12, kCipUsint, &instance->Watchdog_timeout_action               , kGetableSingleAndAll);
    instance->InsertAttribute(13, kCipUint , &instance->Produced_connection_path_length       , kGetableSingleAndAll);
    instance->InsertAttribute(14, kCipEpath, &instance->Produced_connection_path              , kGetableSingleAndAll);
    instance->InsertAttribute(15, kCipUint , &instance->Consumed_connection_path_length       , kGetableSingleAndAll);
    instance->InsertAttribute(16, kCipEpath, &instance->Consumed_connection_path              , kGetableSingleAndAll);
    instance->InsertAttribute(17, kCipUint , &instance->Production_inhibit_time               , kGetableSingleAndAll);
    instance->InsertAttribute(18, kCipUsint, &instance->Connection_timeout_multiplier         , kGetableSingleAndAll);
    instance->InsertAttribute(19, kCipUdint, &instance->Connection_binding_list               , kGetableSingleAndAll);


    object_Set.emplace(object_Set.size(), instance);

    CipStatus stat;
    stat.status = kCipStatusOk;
    stat.extended_status = (CipUsint) instance->id;
    return stat;
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
    const CIP_Connection * conn0, * conn1;
    if ( (conn0 = GetInstance(bound_instances[0])) == nullptr
         | (conn1 = GetInstance(bound_instances[1])) == nullptr)
    {

        //One or both connections don't exist
        status.extended_status = 0x01;
        status.status = kCipErrorResourceUnavailable;
        return status;
    }

    //if both connections exist, then
    //check if there are resources to bound
    if (conn0->Connection_binding_list.num_connections == MAX_BOUND_CONN
         | conn1->Connection_binding_list.num_connections == MAX_BOUND_CONN)
    {
        //Class or instance out of resources to bind
        status.extended_status = 0x02;
        status.status = kCipErrorResourceUnavailable;
        return status;
    }

    if (conn0->State != kConnectionStateEstablished
        | conn1->State != kConnectionStateEstablished)
    {
        //Both instances exist, but at least one is not in Established state
        status.extended_status = 0x01;
        status.status = kCipErrorObjectStateConflict;
        return status;
    }

    if (conn0 == conn1)
    {
        //Both connections are the same
        status.extended_status = 0x01;
        status.status = kCipErrorInvalidParameter;
        return status;
    }

    //check if one or both instances are not dynamically created I/O conn
    //todo: allow I/O conn created statically
    if (false)
    {
        //At least one connection is not dynamically created
        status.extended_status = 0x01;
        status.status = 0xD0;
        return status;
    }

    //check if connections are created internaly and device prevent bind
    //todo: fix this
    if (false)
    {
        //Device prevent binding
        status.extended_status = 0x02;
        status.status = 0xD0;
        return status;
    }

    //all checks passed
    status.status = kCipStatusOk;
    return status;

}

CipStatus CIP_Connection::ProducingLookup(CipEpath *producing_application_path, CipUint *instance_count, std::vector<CipUint> *connection_instance_list)
{
    CipUdint j;
    CipStatus status;
    
    if ( ( j = (CipUdint) object_Set.size() ) < 1)
    {
        status.status = 0x02;
        status.extended_status = 0x01;
        return status;
    }
    
    CIP_Connection * conn;

    //Check every connection to check out if active and producing
    for (CipUdint i = 0; i < j; i++)
    {
        conn = (CIP_Connection*)GetInstance(i);
        if (conn->State == kConnectionStateEstablished)
        {
            if (CipEpath::check_if_equal (producing_application_path, &conn->Produced_connection_path))
            {
                (*instance_count)++;
                connection_instance_list->push_back (conn->id);
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

CipStatus CIP_Connection::InstanceServices(int service, CipMessageRouterRequest_t* msg_router_request, CipMessageRouterResponse_t* msg_router_response)
{
    //Class services
    if (this->id == 0)
    {
        switch (service)
        {/*
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
                */
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
            /*
            case (kConnectionInstServiceBind):
                break;
            case (kConnectionInstServiceProducingLookup):
                break;
            case (kConnectionInstServiceSafetyClose):
                break;
            case (kConnectionInstServiceSafetyOpen):
                break;
                */
            default:
                return kCipStatusError;
        }
        return kCipStatusOk;
    }
}

CipStatus CIP_Connection::Behaviour()
{
    switch (Instance_type)
    {
        case kConnectionTypeExplicit:
            //If explicit connection
            //check direction
            switch (TransportClass_trigger.bitfield_u.direction)
            {
                case kConnectionTriggerDirectionServer:
                    //todo: fix placeholders
                    CipByte * last_msg_ptr, *recv_data_ptr, recv_data_len;
                    CipNotification notification;
                    switch (TransportClass_trigger.bitfield_u.transport_class)
                    {
                        case kConnectionTriggerTransportClass0:
                            //Link_consumer consumes a message and then notifies application
                            Link_consumer->Receive();
                            CIP_MessageRouter::notify_application(Consumed_connection_path, Consumed_connection_path_length, notification);//
                            break;
                        case kConnectionTriggerTransportClass1:
                            //Link_consumer consumes a message, check for duplicates (based on last received sequence count,
                            // notifying the application if dupe happened(and dropping the message), or if the message was received
                            Link_consumer->Receive ();
                            check_for_duplicate(last_msg_ptr, recv_data_ptr) ? notification = kCipNotificationDuplicate : notification = kCipNotificationReceived;
                            CIP_MessageRouter::notify_application(Consumed_connection_path, Consumed_connection_path_length, notification);
                            break;
                        case kConnectionTriggerTransportClass2:
                            //Link_consumer consumes a message, automatically invokes Link_producer, prepending incoming sequence count,
                            // and then delivers the message to the application, that checks for dupes and then do something
                            Link_consumer->Receive();
                            Link_producer->Send();
                            //todo: fix CIP_MessageRouter::route_message(Consumed_connection_path, Consumed_connection_path_length, recv_data_ptr, recv_data_len);//
                            break;
                        case kConnectionTriggerTransportClass3:
                            //Link_consumer consumes a message, then check for dupes and notify the application, that
                            // tells the connection to produce a message with Link_producer, and then send it
                            Link_consumer->Receive();
                            check_for_duplicate(last_msg_ptr, recv_data_ptr) ? notification = kCipNotificationDuplicate : notification = kCipNotificationReceived;
                            //todo: fix CIP_MessageRouter::route_message(Consumed_connection_path, Consumed_connection_path_length, recv_data_ptr, recv_data_len);//
                            break;
                        default:
                            //Unknown transport class, return error
                            //todo: return error
                            break;
                    }
                    break;
                case kConnectionTriggerDirectionClient:
                    switch (TransportClass_trigger.bitfield_u.transport_class)
                    {
                        case kConnectionTriggerTransportClass0:
                            switch(TransportClass_trigger.bitfield_u.production_trigger)
                            {
                                case kConnectionTriggerProductionTriggerCyclic:
                                    break;
                                case kConnectionTriggerProductionTriggerChangeOfState:
                                    break;
                                case kConnectionTriggerProductionTriggerApplicationObj:
                                    break;
                                default:
                                    //Unknown production trigger
                                    break;
                            }
                            break;
                        case kConnectionTriggerTransportClass1:
                            switch(TransportClass_trigger.bitfield_u.production_trigger)
                            {
                                case kConnectionTriggerProductionTriggerCyclic:
                                    break;
                                case kConnectionTriggerProductionTriggerChangeOfState:
                                    break;
                                case kConnectionTriggerProductionTriggerApplicationObj:
                                    break;
                                default:
                                    //Unknown production trigger
                                    break;
                            }
                            break;
                        case kConnectionTriggerTransportClass2:
                            switch(TransportClass_trigger.bitfield_u.production_trigger)
                            {
                                case kConnectionTriggerProductionTriggerCyclic:
                                    break;
                                case kConnectionTriggerProductionTriggerChangeOfState:
                                    break;
                                case kConnectionTriggerProductionTriggerApplicationObj:
                                    break;
                                default:
                                    //Unknown production trigger
                                    break;
                            }
                            break;
                        case kConnectionTriggerTransportClass3:
                            switch(TransportClass_trigger.bitfield_u.production_trigger)
                            {
                                case kConnectionTriggerProductionTriggerCyclic:
                                    break;
                                case kConnectionTriggerProductionTriggerChangeOfState:
                                    break;
                                case kConnectionTriggerProductionTriggerApplicationObj:
                                    break;
                                default:
                                    //Unknown production trigger
                                    break;
                            }
                            break;
                        default:
                            //Unknown transport class, return error
                            //todo: return error
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case kConnectionTypeIo:
            //If IO connection
            break;
        case kConnectionTypeBridged:
            //If bridged connection
            break;
        default:
            //Unknown connection type
            break;

    }
}


bool CIP_Connection::check_for_duplicate(CipByte * last_msg_ptr, CipByte * curr_msg_ptr)
{
    if ( ((CipUint*)last_msg_ptr)[0] == ((CipUint*)curr_msg_ptr)[0])
        return true; //Dupe found
    else
        return false; //Not a dupe
}







