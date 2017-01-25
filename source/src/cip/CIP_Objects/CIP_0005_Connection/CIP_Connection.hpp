//
// Created by gabriel on 25/01/17.
//

#ifndef OPENERMAIN_CIP_CONNECTION_H
#define OPENERMAIN_CIP_CONNECTION_H

#include "../template/CIP_Object.hpp"

class CIP_Connection : public CIP_Object<CIP_Connection>
{
public:
    /** @brief States of a connection */
    typedef enum {
        kConnectionStateNonExistent = 0,
        kConnectionStateConfiguring = 1,
        kConnectionStateWaitingForConnectionId = 2 /**< only used in DeviceNet */,
        kConnectionStateEstablished = 3,
        kConnectionStateTimedOut = 4,
        kConnectionStateDeferredDelete = 5 /**< only used in DeviceNet */,
        kConnectionStateClosing
    } ConnectionState;

/** @brief instance_type attributes */
    typedef enum {
        kConnectionTypeExplicit = 0,
        kConnectionTypeIoExclusiveOwner = 0x01,
        kConnectionTypeIoInputOnly = 0x11,
        kConnectionTypeIoListenOnly = 0x21
    } ConnectionType;

/** @brief Possible values for the watch dog time out action of a connection */
    typedef enum {
        kWatchdogTimeoutActionTransitionToTimedOut = 0, /**< , invalid for explicit message connections */
        kWatchdogTimeoutActionAutoDelete = 1, /**< Default for explicit message connections,
   default for I/O connections on EIP */
        kWatchdogTimeoutActionAutoReset = 2, /**< Invalid for explicit message connections */
        kWatchdogTimeoutActionDeferredDelete = 3 /**< Only valid for DeviceNet, invalid for I/O connections */
    } WatchdogTimeoutAction;

    typedef struct {
        ConnectionState state;
        CipUint connection_id;
        /*TODO think if this is needed anymore
     TCMReceiveDataFunc m_ptfuncReceiveData; */
    } LinkConsumer;

    typedef struct {
        ConnectionState state;
        CipUint connection_id;
    } LinkProducer;

    typedef struct {
        LinkConsumer consumer;
        LinkProducer producer;
    } LinkObject;

    typedef struct {
        CipUsint handle[2];
    }ConnectionHandles;

    ConnectionState state;
    ConnectionType type;
    static CipStatus Init();
    CipStatus InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response)

    //Class services
    static CipStatus Create();
    static CipStatus Delete();
    static CipStatus Reset();
    static CipStatus FindNextInstance();
    static CipStatus GetAttributeSingle();

    //Instance services
    CipStatus Bind(ConnectionHandles connHandle);
    CipStatus ProducingLookup();
    CipStatus SafetyClose();
    CipStatus SafetyOpen();
};


#endif //OPENERMAIN_CIP_CONNECTION_H
