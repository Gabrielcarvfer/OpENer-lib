/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include "CIP_ConnectionManager.hpp"
#include <cstring>
#include <cip/ciptypes.hpp>
#include "../CIP_0001_Identity/CIP_Identity.hpp"
#include "../../connection/network/NET_Endianconv.hpp"
#include "cip/CIP_AppConnType.hpp"
#include "network/NET_NetworkHandler.hpp"
#include "cip/CIP_Objects/CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"

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
        return kCipStatusOk;
    }
    return kCipStatusError;
}


CipStatus CIP_ConnectionManager::Shut()
{
    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}

CipStatus CIP_ConnectionManager::ForwardClose(CipMessageRouterRequest_t* message_router_request,
                                              CipMessageRouterResponse_t* message_router_response)
{


    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}

CipStatus CIP_ConnectionManager::UnconnectedSend(CipMessageRouterRequest_t* message_router_request,
                                                 CipMessageRouterResponse_t* message_router_response)
{
    typedef union
    {
        CipUint val;
        struct
        {
            CipUint redundantOwner:1;
            CipUint connectionType:2;
            CipUint reserved:1;
            CipUint priority:2;
            CipUint fixedOrVariable:1;
            CipUint connectionSize:8;
        }bitfield_u;
    }forwardCloseParams_t;

    forwardCloseParams_t forwardCloseArgs;
    forwardCloseArgs.val = *(CipUint*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipStatusOk;
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
            CipUint redundantOwner:1;
            CipUint connectionType:2;
            CipUint reserved:1;
            CipUint priority:2;
            CipUint fixedOrVariable:1;
            CipUint connectionSize:8;
        }bitfield_u;
    }forwardOpenParams_t;

    forwardOpenParams_t forwardOpenArgs;
    forwardOpenArgs.val = *(CipUint*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}

CipStatus CIP_ConnectionManager::GetConnectionData(CipMessageRouterRequest_t* message_router_request,
                                                   CipMessageRouterResponse_t* message_router_response)
{
    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}

CipStatus CIP_ConnectionManager::SearchConnectionData(CipMessageRouterRequest_t* message_router_request,
                                                      CipMessageRouterResponse_t* message_router_response)
{
    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}

CipStatus CIP_ConnectionManager::GetConnectionOwner(CipMessageRouterRequest_t* message_router_request,
                                                    CipMessageRouterResponse_t* message_router_response)
{
    CipStatus stat;
    stat.status = kCipStatusOk;
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
            CipUdint redundantOwner:1;
            CipUdint connectionType:2;
            CipUdint reserved:1;
            CipUdint priority:2;
            CipUdint fixedOrVariable:1;
            CipUdint reserved2:9;
            CipUdint connectionSize:16;
        }bitfield_u;
    }largeForwardOpenParams_t;

    largeForwardOpenParams_t largeForwardOpenArgs;
    largeForwardOpenArgs.val = *(CipUdint*)&message_router_request->request_data[0];

    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}
