/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <string.h>

#include "cipclass3connection.h"



/**** Implementation ****/
CipStatus EstablishClass3Connection(CIP_Connection* connection_object, CipUint* extended_error)
{
    CipStatus eip_status = kCipStatusOk;
    CipUdint produced_connection_id_buffer;

    /*TODO add check for transport type trigger */
    /* if (0x03 == (g_stDummyCIP_Connection.TransportTypeClassTrigger & 0x03)) */

    CIP_Connection* explicit_connection = GetFreeExplicitConnection();

    if (NULL == explicit_connection)
    {
        eip_status = (CipStatus)kCipErrorConnectionFailure;
        *extended_error = CIP_Connection::kConnectionManagerStatusCodeErrorNoMoreConnectionsAvailable;
    }
    else
    {
        CIP_Connection::CopyConnectionData(explicit_connection, connection_object);

        produced_connection_id_buffer = explicit_connection->produced_connection_id;
        explicit_connection->GeneralConnectionConfiguration();
        explicit_connection->produced_connection_id = produced_connection_id_buffer;
        explicit_connection->instance_type = CIP_Connection::kConnectionTypeExplicit;
        explicit_connection->netConn->SetSocketHandle (kEipInvalidSocket);
        /* set the connection call backs */
        //explicit_connection->connection_close_function = CIP_CIP_Connection::RemoveFromActiveConnections;
        /* explicit connection have to be closed on time out*/
        //explicit_connection->connection_timeout_function = CIP_CIP_Connection::RemoveFromActiveConnections;

        CIP_Connection::AddNewActiveConnection(explicit_connection);
    }
    return eip_status;
}

CIP_Connection* GetFreeExplicitConnection(void)
{
    int i;
    for (i = 0; i < OPENER_CIP_NUM_EXPLICIT_CONNS; i++) {
        if (g_explicit_connections[i]->state == CIP_Connection::kConnectionStateNonExistent)
            return g_explicit_connections[i];
    }
    return NULL;
}

void InitializeClass3ConnectionData(void)
{
    memset(g_explicit_connections, 0,
        OPENER_CIP_NUM_EXPLICIT_CONNS * sizeof(CIP_Connection));
}
