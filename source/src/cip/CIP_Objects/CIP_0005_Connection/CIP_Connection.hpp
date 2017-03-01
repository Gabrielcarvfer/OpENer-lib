//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#ifndef OPENERMAIN_CIP_CONNECTION_H
#define OPENERMAIN_CIP_CONNECTION_H

#include "../template/CIP_Object.hpp"
#include "CIP_Connection_LinkConsumer.hpp"
#include "CIP_Connection_LinkProducer.hpp"
#include <vector>
#include <cip/connection/network/NET_Connection.hpp>

class CIP_Connection : public CIP_Object<CIP_Connection>
{
public:
    /** @brief States of a connection */
    typedef enum {
        kConnectionStateNonExistent = 0,
        kConnectionStateConfiguring = 1,
        kConnectionStateWaitingForConnectionId = 2, // only used in DeviceNet
        kConnectionStateEstablished = 3,
        kConnectionStateTimedOut = 4,
        kConnectionStateDeferredDelete = 5, // only used in DeviceNet
        kConnectionStateClosing = 6
    } ConnectionState;

/** @brief instance_type attributes */
    typedef enum {
        kConnectionTypeExplicit = 0x00,
        kConnectionTypeIo = 0x01,
        kConnectionTypeBridged = 0x02
    } ConnectionType;

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
    typedef enum {
        kConnectionTriggerTransportClass0 = 0,
        kConnectionTriggerTransportClass1 = 1,
        kConnectionTriggerTransportClass2 = 2,
        kConnectionTriggerTransportClass3 = 3,
        kConnectionTriggerTransportClass4 = 4,
        kConnectionTriggerTransportClass5 = 5,
        kConnectionTriggerTransportClass6 = 6,
        kConnectionTriggerProductionTriggerCyclic         = 0,
        kConnectionTriggerProductionTriggerChangeOfState  = 1,
        kConnectionTriggerProductionTriggerApplicationObj = 2,
        kConnectionTriggerProductionTriggerMask           = SET_BIT_N_TO_X (4,1) | SET_BIT_N_TO_X (5,1) | SET_BIT_N_TO_X (6,1),
        kConnectionTriggerDirectionClient                 = 0,
        kConnectionTriggerDirectionServer                 = 1
    } ConnectionTriggerType_e;

    typedef struct
    {
        union
        {
            struct
            {
                CipUsint transport_class    : 4;
                CipUsint production_trigger : 3;
                CipUsint direction          : 1;
            };
            CipUsint val;
        };
    } ConnectionTriggerType_t;

/** @brief Possible values for the watch dog time out action of a connection */
    typedef enum {
        kWatchdogTimeoutActionTransitionToTimedOut = 0, // invalid for explicit message connections
        kWatchdogTimeoutActionAutoDelete = 1, // Default for explicit message connections, default for I/O connections on EIP
        kWatchdogTimeoutActionAutoReset = 2, // Invalid for explicit message connections
        kWatchdogTimeoutActionDeferredDelete = 3 // Only valid for DeviceNet, invalid for I/O connections
    } WatchdogTimeoutAction;

    //Instance attributes (ids 1 to 19)
    ConnectionState State;
    ConnectionType Instance_type;
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
    std::vector<CipUint> Connection_binding_list;

    //Links
    CIP_Connection_LinkConsumer * Link_consumer;
    CIP_Connection_LinkProducer * Link_producer;

    static CipStatus Init();
    CipStatus InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response);

    //Class services
    static CipStatus Create();
    static CipStatus Delete();
    static CipStatus Reset();
    static CipStatus FindNextInstance();
    static CipStatus GetAttributeSingle();


    static CipStatus Bind(CipUint bound_instances[2]);
    static CipStatus ProducingLookup(CipEpath producing_application_path, CipUint *instance_count, std::vector<CipUint> connection_instance_list[]);
    static CipStatus SafetyClose();
    static CipStatus SafetyOpen();

private:
    //temporary
    CipStatus Behaviour();
    NET_Connection * netConn;
    bool check_for_duplicate(CipByte * last_msg_ptr, CipByte * curr_msg_ptr);
};


#endif //OPENERMAIN_CIP_CONNECTION_H
