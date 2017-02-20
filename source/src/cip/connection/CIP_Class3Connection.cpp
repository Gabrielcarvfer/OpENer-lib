/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
//Includes
#include <cstring>
#include "CIP_Class3Connection.hpp"



//Methods
CipStatus CIP_Class3conn::EstablishClass3Connection(CipUint* extended_error)
{
    CipStatus eip_status = kCipStatusOk;
    CipUdint produced_connection_id_buffer;

    /*TODO add check for transport type trigger */
    /* if (0x03 == (g_stDummyCIP_Connection.TransportTypeClassTrigger & 0x03)) */

    CIP_Connection* explicit_connection = GetFreeExplicitConnection();

    if (nullptr == explicit_connection)
    {
        eip_status = (CipStatus)kCipErrorConnectionFailure;
        *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorNoMoreConnectionsAvailable;
    }
    else
    {
        CIP_ConnectionManager::CopyConnectionData(explicit_connection, this);

        produced_connection_id_buffer = explicit_connection->CIP_produced_connection_id;
        CIP_ConnectionManager::GeneralConnectionConfiguration (explicit_connection);
        explicit_connection->CIP_produced_connection_id = produced_connection_id_buffer;
        explicit_connection->Instance_type = kConnectionTypeExplicit;
        explicit_connection->netConn->SetSocketHandle (kEipInvalidSocket);
        /* set the connection call backs */
        //explicit_connection->connection_close_function = CIP_ConnectionManager::RemoveFromActiveConnections;
        /* explicit connection have to be closed on time out*/
        //explicit_connection->connection_timeout_function = CIP_ConnectionManager::RemoveFromActiveConnections;

        CIP_ConnectionManager::AddNewActiveConnection(explicit_connection);
    }
    return eip_status;
}

CIP_Connection * CIP_Class3conn::GetFreeExplicitConnection(void)
{
    int i;
    for (i = 0; i < OPENER_CIP_NUM_EXPLICIT_CONNS; i++)
    {
        if (g_explicit_connections[i]->State == kConnectionStateNonExistent)
            return g_explicit_connections[i];
    }
    return nullptr;
}

void CIP_Class3conn::InitializeClass3ConnectionData(void)
{
    //memset(g_explicit_connections, 0, OPENER_CIP_NUM_EXPLICIT_CONNS * sizeof(CIP_ConnectionManager));
    //Static variables
    //CIP_ConnectionManager * CIP_Class3conn::g_explicit_connections[OPENER_CIP_NUM_EXPLICIT_CONNS];
}
