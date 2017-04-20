/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include "CIP_ConnectionManager.hpp"
#include <cstring>

std::map<CipUdint, const CIP_ConnectionManager *> *CIP_ConnectionManager::active_connections_set;
CipUdint CIP_ConnectionManager::g_incarnation_id;

/** @brief Generate a new connection Id utilizing the Incarnation Id as
 * described in the EIP specs.
 *
 * A unique connectionID is formed from the boot-time-specified "incarnation ID"
 * and the per-new-connection-incremented connection number/counter.
 * @return new connection id
 */
CipUdint CIP_ConnectionManager::GetConnectionId (void)
{
    static CipUdint connection_id = 18;
    connection_id++;
    return (g_incarnation_id | (connection_id & 0x0000FFFF));
}

CipStatus CIP_ConnectionManager::Init ()
{
    CipStatus status;
    if (number_of_instances == 0)
    {

        class_id = kCipConnectionManagerClassCode;
        class_name = "Connection Manager";
        revision = 1;
        max_instances = 1;
        maximum_id_number_class_attributes = 8;
        maximum_id_number_instance_attributes = 13;

        CIP_ConnectionManager *instance = new CIP_ConnectionManager();
        object_Set.emplace(object_Set.size(), instance);

        //g_incarnation_id = ((CipUdint) unique_connection_id) << 16;
        return kCipGeneralStatusCodeSuccess;
    }
    return kCipStatusError;
}


CipStatus CIP_ConnectionManager::Shut()
{
    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::ForwardClose(CipMessageRouterRequest_t* message_router_request,
                                              CipMessageRouterResponse_t* message_router_response)
{
    typedef struct
    {
        CipUsint PriorityNtimeTick;
        CipUsint TimeoutTicks;
        CipUint  ConnectionSerialNumber;
        CipUint  OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
        CipUsint ConnectionPathSize; //number of 16bit words of conn_path
        CipUsint Reserved;
        CipEpath ConnectionPath;     //padded epath indicate route to remote target device
    }forwardCloseRequestParams_t;

    forwardCloseRequestParams_t *forwardCloseArgs;
    forwardCloseArgs = (forwardCloseRequestParams_t*)&message_router_request->request_data[0];

    //sucessfull if conn triad matches an existing connection parameters
    //todo: free structures
    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;


    //if no match, no conn is released and target shall return an errror with gen status code 0x01 and extended status 0x0107
    // (target conn not found) unless device detercted an improper conn path. Conn path size may be ignored, but improper conn path
    // mismatch may cause service to be unsucessful and return error regardless of a conn triad match

    //improper format
    // conn_path_size < conn_path -> return gen status 0x15 (too much data in service request)
    // conn_path_size > conn_path -> return gen status 0x13 (not enough data in service request)

    //conn_path mismatch
    // mismatch between conn_path value received and sent in originating conn_request, return gen status 0x01
    //   and extended of 0x316 (error in forward close service connection path mismatch ) or 0x0315 (invalid segment in connection path)


    typedef struct
    {
        CipUint  ConnectionSerialNumber;
        CipUint  OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
        CipUsint ApplicationReplySize;
        CipUsint Reserved;
        std::vector<CipUsint> ApplicationReply;
    } forwardCloseResponseParamsSuccessful_t;

    typedef struct
    {
        CipUint  ConnectionSerialNumber;
        CipUint  OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
        CipUsint RemainingPathSize;
        CipUsint Reserved;
    } forwardCloseResponseParamsUnsuccessful_t;


    return stat;
}


/**
  * Used to send messages between nodes without a proper established connection
  * UCMM of each node send the message forward requesting other Unconnected Send
  * (when an intermediate node removes the last port segment, the embedded message reqeust shall be formatted
  *     as a message router request message and sent to the port and link address of the last port segment using the UCMM for that link type
  *     The target node receives only the embedded message request arriving via the UCMM)
  * The Electronic key segment is not used in UnconnectedSend requests
  */
CipStatus CIP_ConnectionManager::UnconnectedSend(CipMessageRouterRequest_t* message_router_request,
                                                 CipMessageRouterResponse_t* message_router_response)
{
    typedef struct
    {
        CipUsint PriorityNtimeTick;
        CipUsint TimeoutTicks;
        CipUint EmbeddedMessageRequestSize; // num bytes of embedded message request
        struct
        {
            CipUsint Service;
            CipUsint RequestPathSize;
            CipEpath RequestPath;
            std::vector<CipUsint> RequestData;
        }EmbeddedMessageRequest;
        CipUsint Pad;           //only preset if embedded message rouute request size is odd
        CipUsint RoutePathSize; //# of 16bit-words in routePath
        CipUsint Reserved;
        CipEpath RoutePath;     // route to remote target (electronic key segment) + port segment
    } unconnectedSendRequestParams_t;

    //if successful
    // return in service response data an array of bytes containing service return (empty if service/object doesn't return anything)
    //else
    // return usint with remaining path size (indicate number of words in original routePath, as seen by node that detects the error) plus reserved byte zeroed

    unconnectedSendRequestParams_t *unconnectedSendArgs;
    unconnectedSendArgs = (unconnectedSendRequestParams_t*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::ForwardOpen(CipMessageRouterRequest_t* message_router_request,
                                             CipMessageRouterResponse_t* message_router_response)
{
    typedef union
    {
        CipUint val;
        struct
        {
            CipUint redundantOwner :1;
            CipUint connectionType :2;
            CipUint reserved       :1;
            CipUint priority       :2;
            CipUint fixedOrVariable:1;
            CipUint connectionSize :8;
        }bitfield_u;
    }forwardOpenParams_t;

    forwardOpenParams_t *forwardOpenArgs;
    forwardOpenArgs = (forwardOpenParams_t*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::GetConnectionData(CipMessageRouterRequest_t* message_router_request,
                                                   CipMessageRouterResponse_t* message_router_response)
{

    typedef struct
    {
        CipUint ConnectionNumber;
    } getConnectionDataRequestParams_t;

    getConnectionDataRequestParams_t *getConnectionDataArgs;
    getConnectionDataArgs = (getConnectionDataRequestParams_t*)&message_router_request->request_data[0];

    typedef struct
    {
        CipUint ConnectionNumber;
        CipUint ConnectionState;
        CipUint OriginatorPort;
        CipUint TargetPort;
        CipUint ConnectionSerialNumber;
        CipUint OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
        CipUdint Originator_O_to_T_CID;
        CipUdint Target_O_to_T_CID;
        CipUsint ConnectionTimeoutMultiplier1;
        CipUsint Reserved1[3];
        CipUdint OriginatorRPI_O_to_T;
        CipUdint OriginatorAPI_O_to_T;
        CipUdint Originator_T_to_O_CID;
        CipUdint Target_T_to_O_CID;
        CipUsint ConnectionTimeoutMultiplier2;
        CipUsint Reserved2[3];
        CipUdint OriginatorRPI_T_to_O;
        CipUdint OriginatorAPI_T_to_O;
    } getConnectionDataResponseParams_t;

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::SearchConnectionData(CipMessageRouterRequest_t* message_router_request,
                                                      CipMessageRouterResponse_t* message_router_response)
{

    typedef struct
    {
        CipUint  ConnectionSerialNumber;
        CipUint  OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
    } searchConnectionDataRequestParams_t;

    searchConnectionDataRequestParams_t * searchConnectionDataArgs;
    searchConnectionDataArgs = (searchConnectionDataRequestParams_t*)&message_router_request->request_data[0];

    //Response format = getConnectionDataResponseParams_t
    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::GetConnectionOwner(CipMessageRouterRequest_t* message_router_request,
                                                    CipMessageRouterResponse_t* message_router_response)
{
    typedef struct
    {
        CipUsint reserved; // zeroed
        CipUsint ConnectionPathSize;
        CipEpath ConnectionPath;
    } getConnectionOwnerRequestParams_t;

    getConnectionOwnerRequestParams_t *getConnectionOwnerArgs;
    getConnectionOwnerArgs = (getConnectionOwnerRequestParams_t*)&message_router_request->request_data[0];

    typedef struct
    {
        CipUsint NumberOfConnections;
        CipUsint NumberClaimingOwnership;
        CipUsint NumberReadyForOwnership;
        CipUsint LastAction;
        CipUsint ConnectionSerialNumber;
        CipUint  OriginatorVendorID;
        CipUdint OriginatorSerialNumber;
    } getConnectionOwnerResponseSuccessful_t;

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_ConnectionManager::LargeForwardOpen(CipMessageRouterRequest_t* message_router_request,
                                                  CipMessageRouterResponse_t* message_router_response)
{
    typedef union
    {
        CipUdint val;
        struct
        {
            CipUdint redundantOwner :1 ;
            CipUdint connectionType :2 ;
            CipUdint reserved       :1 ;
            CipUdint priority       :2 ;
            CipUdint fixedOrVariable:1 ;
            CipUdint reserved2      :9 ;
            CipUdint connectionSize :16;
        }bitfield_u;
    }largeForwardOpenParams_t;

    largeForwardOpenParams_t *largeForwardOpenArgs;
    largeForwardOpenArgs = (largeForwardOpenParams_t*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}


void * CIP_ConnectionManager::retrieveAttribute(CipUsint attributeNumber)
{

}

CipStatus CIP_ConnectionManager::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{

}