//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#include <typedefs.hpp>
#include <vector>
#include <cstring>
#include <cip/CIP_Objects/CIP_0002_MessageRouter/CIP_MessageRouter.hpp>
#include <cip/ciptypes.hpp>
#include "CIP_Connection.hpp"

CipStatus CIP_Connection::Init()
{
    if (number_of_instances == 0)
    {
        class_id = 5;
        class_name = "Connection";
        revision = 1;
        max_instances = 10;

        RegisterGenericClassAttributes();
        //Chapter 5 vol 5
        //todo: recheck sizes
        //classAttrInfo.emplace(8, kCipUint, &ConnectionRequestErrorCount, kGetableSingleAndAll));
        //classAttrInfo.emplace(9, kCipUint, &SafetyConnectionCounters   , kGetableSingleAndAll));


        CIP_Connection *instance = new CIP_Connection();
        AddClassInstance(instance,0);

		//Setup instances attributes
		//Chapter 3-4.4 vol 1
		instAttrInfo.emplace( 1, CipAttrInfo_t{ kCipUsint, sizeof( CipUsint), kGetableSingleAndAll, "State"                                 });
		instAttrInfo.emplace( 2, CipAttrInfo_t{ kCipUsint, sizeof( CipUsint), kGetableSingleAndAll, "Instance_type"                         });
		instAttrInfo.emplace( 3, CipAttrInfo_t{ kCipByte , sizeof( CipByte ), kGetableSingleAndAll, "TransportClass_trigger"                });
		instAttrInfo.emplace( 4, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "DeviceNet_produced_connection_id"      });
		instAttrInfo.emplace( 5, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "DeviceNet_consumed_connection_id"      });
		instAttrInfo.emplace( 6, CipAttrInfo_t{ kCipByte , sizeof( CipByte ), kGetableSingleAndAll, "DeviceNet_initial_comm_characteristics"});
		instAttrInfo.emplace( 7, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Produced_connection_size"              });
		instAttrInfo.emplace( 8, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Consumed_connection_size"              });
		instAttrInfo.emplace( 9, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Expected_packet_rate"                  });
		instAttrInfo.emplace(10, CipAttrInfo_t{ kCipUdint, sizeof( CipUdint), kGetableSingleAndAll, "CIP_produced_connection_id"            });
		instAttrInfo.emplace(11, CipAttrInfo_t{ kCipUdint, sizeof( CipUdint), kGetableSingleAndAll, "CIP_consumed_connection_id"            });
		instAttrInfo.emplace(12, CipAttrInfo_t{ kCipUsint, sizeof( CipUsint), kGetableSingleAndAll, "Watchdog_timeout_action"               });
		instAttrInfo.emplace(13, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Produced_connection_path_length"       });
		instAttrInfo.emplace(14, CipAttrInfo_t{ kCipEpath, sizeof( CipEpath), kGetableSingleAndAll, "Produced_connection_path"              });
		instAttrInfo.emplace(15, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Consumed_connection_path_length"       });
		instAttrInfo.emplace(16, CipAttrInfo_t{ kCipEpath, sizeof( CipEpath), kGetableSingleAndAll, "Consumed_connection_path"              });
		instAttrInfo.emplace(17, CipAttrInfo_t{ kCipUint , sizeof( CipUint ), kGetableSingleAndAll, "Production_inhibit_time"               });
		instAttrInfo.emplace(18, CipAttrInfo_t{ kCipUsint, sizeof( CipUsint), kGetableSingleAndAll, "Connection_timeout_multiplier"         });
		instAttrInfo.emplace(19, CipAttrInfo_t{ kCipUdint, sizeof( CipUdint), kGetableSingleAndAll, "Connection_binding_list"               });

        //Class services
        classServicesProperties.emplace(kConnectionServiceCreate                      , CipServiceProperties_t{ "Create"            });
        classServicesProperties.emplace(kConnectionClassNInstServiceDelete            , CipServiceProperties_t{ "Delete"            });
        classServicesProperties.emplace(kConnectionClassNInstServiceReset             , CipServiceProperties_t{ "Reset"             });
        classServicesProperties.emplace(kConnectionServiceFindNextInstance            , CipServiceProperties_t{ "FindNextInstance"  });
        classServicesProperties.emplace(kConnectionClassNInstServiceGetAttributeSingle, CipServiceProperties_t{ "GetAttributeSingle"});
        classServicesProperties.emplace(kConnectionServiceBind                        , CipServiceProperties_t{ "Bind"              });
        classServicesProperties.emplace(kConnectionServiceProducingLookup             , CipServiceProperties_t{ "ProducingLookup"   });
        classServicesProperties.emplace(kConnectionServiceSafetyClose                 , CipServiceProperties_t{ "SafetyClose"       });
        classServicesProperties.emplace(kConnectionServiceSafetyOpen                  , CipServiceProperties_t{ "SafetyOpen"        });

    }
    return kCipGeneralStatusCodeSuccess;
}
//Class services
CipStatus CIP_Connection::Create(CipMessageRouterRequest_t* message_router_request,
                                 CipMessageRouterResponse_t* message_router_response)
{
    CipStatus stat;
    CIP_Connection *instance;
    try
    {
         instance = new CIP_Connection();
    }
    catch (const std::range_error& error)
    {
        stat.status = kCipStatusError;
        return stat;
    }
    AddClassInstance(instance,-1);
    stat.status = kCipStatusOk;
    stat.extended_status = (CipUsint) instance->id;
    return stat;
}

CipStatus CIP_Connection::ApplyAttributes(CipMessageRouterResponse_t* message_router_request,
                          CipMessageRouterResponse_t* message_router_response)
{

}

CipStatus CIP_Connection::Delete(CipMessageRouterRequest_t* message_router_request,
                                 CipMessageRouterResponse_t* message_router_response)
{
    if (this->id == 0)
    {
        //If class, close and delete all connections other than the class instance
        for (int i = 1; i < object_Set.size(); i++)
        {
            delete object_Set[i];
            object_Set.erase(i);
        }
    }
    else
    {
        //If instance, kill itself
        delete this;
    }
}

CipStatus CIP_Connection::Reset(CipMessageRouterRequest_t* message_router_request,
                                CipMessageRouterResponse_t* message_router_response)
{

    if (this->id == 0)
    {
        //The class instance resets all instances that are resettable
        for (int i = 1; i < object_Set.size(); i++)
        {
            // In case of I/O conns, for Established and Timed out
            if ( object_Set[i]->Instance_type == kConnectionTypeIo
                 && (object_Set[i]->State == kConnectionStateEstablished
                     || object_Set[i]->State == kConnectionStateTimedOut) )
            {
                object_Set[i]->State = kConnectionStateEstablished;
            }

            // In case of explicit conns, Established and Deferred Delete
            if ( object_Set[i]->Instance_type == kConnectionTypeExplicit
                    && (object_Set[i]->State == kConnectionStateEstablished
                        || object_Set[i]->State == kConnectionStateDeferredDelete) )
            {
                object_Set[i]->State = kConnectionStateEstablished;
            }

            // In case of bridged conns, there's no reset
        }
    }

}

CipStatus CIP_Connection::FindNextInstance(CipMessageRouterRequest_t* message_router_request,
                                           CipMessageRouterResponse_t* message_router_response)
{

}

//Instance services
CipStatus CIP_Connection::Bind(CipMessageRouterRequest_t* message_router_request,
                               CipMessageRouterResponse_t* message_router_response)//(CipUint bound_instances[2])
{
    typedef struct
    {
        CipUint bound_instances[2];
    }bindRequestParams_t;

    bindRequestParams_t * bindArgs;
    bindArgs = (bindRequestParams_t*)&message_router_request->request_data[0];

    CipStatus status;
    CIP_Connection * conn0 = nullptr, * conn1 = nullptr;
    conn0 = (CIP_Connection*)GetInstance(bindArgs->bound_instances[0]);
    conn1 = (CIP_Connection*)GetInstance(bindArgs->bound_instances[1]);

    if ( (conn0 == nullptr) | (conn1 == nullptr))
    {
        //One or both connections don't exist
        status.extended_status = 0x01;
        status.status = kCipGeneralStatusCodeResourceUnavailable;
        return status;
    }

    //if both connections exist, then
    //check if there are resources to bound
    if (conn0->Connection_binding_list.num_connections == MAX_BOUND_CONN
         | conn1->Connection_binding_list.num_connections == MAX_BOUND_CONN)
    {
        //Class or instance out of resources to bind
        status.extended_status = 0x02;
        status.status = kCipGeneralStatusCodeResourceUnavailable;
        return status;
    }

    if (conn0->State != kConnectionStateEstablished
        | conn1->State != kConnectionStateEstablished)
    {
        //Both instances exist, but at least one is not in Established state
        status.extended_status = 0x01;
        status.status = kCipGeneralStatusCodeObjectStateConflict;
        return status;
    }

    if (conn0 == conn1)
    {
        //Both connections are the same
        status.extended_status = 0x01;
        status.status = kCipGeneralStatusCodeInvalidParameter;
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

    //all checks passed, add both connections to each other binding lists

    (conn0->Connection_binding_list.num_connections)++;
    (conn1->Connection_binding_list.num_connections)++;

    conn0->Connection_binding_list.connections_list.push_back(conn1->id);
    conn1->Connection_binding_list.connections_list.push_back(conn0->id);

    status.status = kCipGeneralStatusCodeSuccess;
    return status;

}

CipStatus CIP_Connection::ProducingLookup(CipMessageRouterRequest_t* message_router_request,
                                          CipMessageRouterResponse_t* message_router_response)
//(CipEpath *producing_application_path, CipUint *instance_count, std::vector<CipUint> *connection_instance_list)
{
    CipUdint j;
    CipStatus status;

    //Router request fields
    CipUint instance_count = 0;
    std::vector<CipUint> connection_instance_list;
    CipEpath * producing_application_path = (CipEpath *)&message_router_request->request_data[0];
    
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
                instance_count++;
                connection_instance_list.push_back (conn->id);
            }
        }
    }

    //write instance count and conn_instance_list back to router response
   message_router_response->response_data.push_back(((CipUsint*)&instance_count)[0]);
   message_router_response->response_data.push_back(((CipUsint*)&instance_count)[1]);

    for (CipUdint i = 0; i < connection_instance_list.size(); i++)
    {
       message_router_response->response_data.push_back(((CipUsint*)&connection_instance_list)[0]);
       message_router_response->response_data.push_back(((CipUsint*)&connection_instance_list)[1]);
    }

    status.status = 0x0;
    status.extended_status = 0x0;
    return status;
}

CipStatus CIP_Connection::SafetyClose(CipMessageRouterRequest_t* message_router_request,
                                      CipMessageRouterResponse_t* message_router_response)
{

}

CipStatus CIP_Connection::SafetyOpen(CipMessageRouterRequest_t* message_router_request,
                                     CipMessageRouterResponse_t* message_router_response)
{

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

                        case kConnectionTriggerTransportClass1:
                            //Link_consumer consumes a message, check for duplicates (based on last received sequence count,
                            // notifying the application if dupe happened(and dropping the message), or if the message was received

                            Link_consumer->Receive ();
                            check_for_duplicate(last_msg_ptr, recv_data_ptr) ? notification = kCipNotificationDuplicate : notification = kCipNotificationReceived;
                            CIP_MessageRouter::notify_application(Consumed_connection_path, Consumed_connection_path_length, notification);

                            //todo: implement class 1 server behaviour

                            break;
                        case kConnectionTriggerTransportClass2:
                            //Link_consumer consumes a message, automatically invokes Link_producer, prepending incoming sequence count,
                            // and then delivers the message to the application, that checks for dupes and then do something

                            Link_consumer->Receive();
                            Link_producer->Send();
                            //todo: fix CIP_MessageRouter::route_message(Consumed_connection_path, Consumed_connection_path_length, recv_data_ptr, recv_data_len);//

                            //todo: implement class 2 server behaviour


                            //todo: implement class 2 server behaviour

                            break;
                        case kConnectionTriggerTransportClass3:
                            //Link_consumer consumes a message, then check for dupes and notify the application, that
                            // tells the connection to produce a message with Link_producer, and then send it
                           
                            Link_consumer->Receive();
                            check_for_duplicate(last_msg_ptr, recv_data_ptr) ? notification = kCipNotificationDuplicate : notification = kCipNotificationReceived;
                            //todo: fix CIP_MessageRouter::route_message(Consumed_connection_path, Consumed_connection_path_length, recv_data_ptr, recv_data_len);//

                            //todo: implement class 3 server behaviour

                            //todo: implement class 3 server behaviour
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

CipStatus CIP_Connection::Shut()
{
    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;

}



void * CIP_Connection::retrieveAttribute(CipUsint attributeNumber)
{
	if (this->id == 0)
	{
		switch (attributeNumber)
		{
		 case 1: return &revision;
		 case 2: return &max_instances;
		 case 3: return &number_of_instances;
		 case 4: return &optional_attribute_list;
		 case 5: return &optional_service_list;
		 case 6: return &maximum_id_number_class_attributes;
		 case 7: return &maximum_id_number_instance_attributes;
		 default: return nullptr;
		}
	}
	else
	{
		switch (attributeNumber)
		{
		case  1: return &this->State;
		case  2: return &this->Instance_type;
		case  3: return &this->TransportClass_trigger;
		case  4: return &this->DeviceNet_produced_connection_id;
		case  5: return &this->DeviceNet_consumed_connection_id;
		case  6: return &this->DeviceNet_initial_comm_characteristics;
		case  7: return &this->Produced_connection_size;
		case  8: return &this->Consumed_connection_size;
		case  9: return &this->Expected_packet_rate;
		case 10: return &this->CIP_produced_connection_id;
		case 11: return &this->CIP_consumed_connection_id;
		case 12: return &this->Watchdog_timeout_action;
		case 13: return &this->Produced_connection_path_length;
		case 14: return &this->Produced_connection_path;
		case 15: return &this->Consumed_connection_path_length;
		case 16: return &this->Consumed_connection_path;
		case 17: return &this->Production_inhibit_time;
		case 18: return &this->Connection_timeout_multiplier;
		case 19: return &this->Connection_binding_list;
		default: return nullptr;
		}
	}
}

CipStatus CIP_Connection::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{
    switch(serviceNumber)
    {
        case kConnectionServiceCreate                      : return this->Create(req,resp);
        case kConnectionClassNInstServiceDelete            : return this->Delete(req,resp);
        case kConnectionClassNInstServiceReset             : return this->Reset(req,resp);
        case kConnectionServiceFindNextInstance            : return this->FindNextInstance(req,resp);
        case kConnectionClassNInstServiceGetAttributeSingle: return this->GetAttributeSingle(req,resp);
        case kConnectionServiceBind                        : return this->Bind(req,resp);
        case kConnectionServiceProducingLookup             : return this->ProducingLookup(req,resp);
        case kConnectionServiceSafetyClose                 : return this->SafetyClose(req,resp);
        case kConnectionServiceSafetyOpen                  : return this->SafetyOpen(req,resp);
        case kConnectionInstSetAttributeSingle             : return this->SetAttributeSingle(req,resp);

    }
}










