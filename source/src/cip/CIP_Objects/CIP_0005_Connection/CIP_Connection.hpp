//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#ifndef CIP_CLASSES_CONNECTION_H
#define CIP_CLASSES_CONNECTION_H

#include "cip/CIP_Objects/template/CIP_Object_template.hpp"
#include "CIP_Connection_LinkConsumer.hpp"
#include "CIP_Connection_LinkProducer.hpp"
#include <vector>
#include "../../connection/network/NET_Connection.hpp"


#define MAX_BOUND_CONN 10
class CIP_Connection;
class CIP_Connection : public CIP_Object_template<CIP_Connection>
{
public:
    /** @brief States of a connection */
    typedef enum
    {
        kConnectionStateNonExistent    = 0,
        kConnectionStateConfiguring    = 1,
        kConnectionStateWaitingForConnectionId = 2, // only used in DeviceNet
        kConnectionStateEstablished    = 3,
        kConnectionStateTimedOut       = 4,
        kConnectionStateDeferredDelete = 5, // only used in DeviceNet
        kConnectionStateClosing        = 6
    } ConnectionState_e;

/** @brief instance_type attributes */
    typedef enum
    {
        kConnectionTypeExplicit = 0,
        kConnectionTypeIo       = 1,
        kConnectionTypeBridged  = 2
    } ConnectionType_e;

    typedef enum {
        kConnectionServiceCreate           = 0x08,
        kConnectionClassNInstServiceDelete = 0x09,
        kConnectionClassNInstServiceReset  = 0x05,
        kConnectionServiceFindNextInstance = 0x11,
        kConnectionClassNInstServiceGetAttributeSingle = 0x0E,
        kConnectionServiceBind             = 0x4B,
        kConnectionServiceProducingLookup  = 0x4C,
        kConnectionServiceSafetyClose      = 0x4E,
        kConnectionServiceSafetyOpen       = 0x54,
        kConnectionInstSetAttributeSingle  = 0x10,
    } ConnectionServices_e;

    /** #brief transportClass_trigger
     *  byte\bits   7           6   5   4           3   2   1   0
     *      1       Direction   ProductionTrigger   Transport Class
     *
     *  Direction (0 = client, 1 = server)
     *  Production trigger (0 = cyclic, 1 = change of state, 2 = application object)
     *  Transport Class ( x = class x)
     *  Transports 0 and 1 - consumer/producer only, based on Dir, launches LinkProducer/Consumer
     *  Transports 2 and 3 - produce and consume on same connection (request by client to server, response from server to client)
     *  Transport 4 - nonblocking
     *  Transport 5 - nonblocking & fragmented
     *  Transport 6 - multicast & fragmented
     */
    typedef enum
    {
        kConnectionTriggerTransportClass0 = 0,
        kConnectionTriggerTransportClass1 = 1,
        kConnectionTriggerTransportClass2 = 2,
        kConnectionTriggerTransportClass3 = 3,
        kConnectionTriggerTransportClass4 = 4,
        kConnectionTriggerTransportClass5 = 5,
        kConnectionTriggerTransportClass6 = 6
    }connection_trigger_transport_class_e;

    typedef enum
    {
        kConnectionTriggerProductionTriggerCyclic         = 0,
        kConnectionTriggerProductionTriggerChangeOfState  = 1,
        kConnectionTriggerProductionTriggerApplicationObj = 2
    }connection_trigger_transport_production_e;

    typedef enum
    {
        kConnectionTriggerDirectionClient = 0,
        kConnectionTriggerDirectionServer = 1
    }connection_trigger_transport_direction_e;

    typedef union
    {
        CipUsint val;
        struct
        {
            CipUsint transport_class    : 4;
            CipUsint production_trigger : 3;
            CipUsint direction          : 1;
        }bitfield_u;


    } ConnectionTriggerType_t;

    typedef struct
    {
        CipUint num_connections;
        std::vector<CipUint> connections_list;
    }Connection_binding_list_t;


/** @brief Possible values for the watch dog time out action of a connection */
    typedef enum
    {
        kWatchdogTimeoutActionTransitionToTimedOut = 0, // invalid for explicit message connections
        kWatchdogTimeoutActionAutoDelete = 1, // Default for explicit message connections, default for I/O connections on EIP
        kWatchdogTimeoutActionAutoReset = 2, // Invalid for explicit message connections
        kWatchdogTimeoutActionDeferredDelete = 3 // Only valid for DeviceNet, invalid for I/O connections
    } WatchdogTimeoutAction;

    //Instance attributes (ids 1 to 19)
    ConnectionState_e State;
    ConnectionType_e Instance_type;
    ConnectionTriggerType_t TransportClass_trigger;
    CipUint DeviceNet_produced_connection_id;
    CipUint DeviceNet_consumed_connection_id;
    CipByte DeviceNet_initial_comm_characteristics;
    CipUint Produced_connection_size;
    CipUint Consumed_connection_size;
    CipUint Expected_packet_rate;
    CipUdint CIP_produced_connection_id;
    CipUdint CIP_consumed_connection_id;
    WatchdogTimeoutAction Watchdog_timeout_action;
    CipUint Produced_connection_path_length;
    CipEpath Produced_connection_path;
    CipUint Consumed_connection_path_length;
    CipEpath Consumed_connection_path;
    CipUint Production_inhibit_time;
    CipUsint Connection_timeout_multiplier;
    Connection_binding_list_t Connection_binding_list;

    //Links
    CIP_Connection_LinkConsumer * Link_consumer;
    CIP_Connection_LinkProducer * Link_producer;

    static CipStatus Init();
    static CipStatus Shut();

    //Class services
    static CipStatus Create(CipMessageRouterRequest_t* message_router_request,
                            CipMessageRouterResponse_t* message_router_response);

    static CipStatus Delete(CipMessageRouterRequest_t* message_router_request,
                            CipMessageRouterResponse_t* message_router_response);

    static CipStatus Reset(CipMessageRouterRequest_t* message_router_request,
                           CipMessageRouterResponse_t* message_router_response);

    static CipStatus FindNextInstance(CipMessageRouterRequest_t* message_router_request,
                                      CipMessageRouterResponse_t* message_router_response);

    static CipStatus ApplyAttributes(CipMessageRouterResponse_t* message_router_request,
                              CipMessageRouterResponse_t* message_router_response);

    static CipStatus Bind(CipMessageRouterRequest_t* message_router_request,
                          CipMessageRouterResponse_t* message_router_response);//CipUint bound_instances[2]);

    static CipStatus ProducingLookup(CipMessageRouterRequest_t* message_router_request,
                                     CipMessageRouterResponse_t* message_router_response);//CipEpath * producing_application_path, CipUint *instance_count, std::vector<CipUint> *connection_instance_list);

    static CipStatus SafetyClose(CipMessageRouterRequest_t* message_router_request,
                                 CipMessageRouterResponse_t* message_router_response);

    static  CipStatus SafetyOpen(CipMessageRouterRequest_t* message_router_request,
                                CipMessageRouterResponse_t* message_router_response);


    //Temporary
    NET_Connection * netConn;
private:
    CipStatus Behaviour();
    bool check_for_duplicate(CipByte * last_msg_ptr, CipByte * curr_msg_ptr);
	void * retrieveAttribute(CipUsint attributeNumber);
	CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
};


#endif //CIP_CLASSES_CONNECTION_H
