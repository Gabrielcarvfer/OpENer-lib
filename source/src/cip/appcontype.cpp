/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include "appcontype.h"
#include "src/cip/connection_stack/CIP_Connection.h"
#include "opener_api.h"
#include <string.h>

typedef struct {
    unsigned int output_assembly; /**< the O-to-T point for the connection */
    unsigned int input_assembly; /**< the T-to-O point for the connection */
    unsigned int config_assembly; /**< the config point for the connection */
    CIP_Connection *connection_data; /**< the connection data, only one connection is allowed per O-to-T point*/
} ExclusiveOwnerConnection;

typedef struct {
    unsigned int output_assembly; /**< the O-to-T point for the connection */
    unsigned int input_assembly; /**< the T-to-O point for the connection */
    unsigned int config_assembly; /**< the config point for the connection */
    CIP_Connection *connection_data[OPENER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH]; /*< the connection data */
} InputOnlyConnection;

typedef struct {
    unsigned int output_assembly; /**< the O-to-T point for the connection */
    unsigned int input_assembly; /**< the T-to-O point for the connection */
    unsigned int config_assembly; /**< the config point for the connection */
    CIP_Connection *connection_data[OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH]; /**< the connection data */
} ListenOnlyConnection;

ExclusiveOwnerConnection g_exlusive_owner_connections[OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS];

InputOnlyConnection g_input_only_connections[OPENER_CIP_NUM_INPUT_ONLY_CONNS];

ListenOnlyConnection g_listen_only_connections[OPENER_CIP_NUM_LISTEN_ONLY_CONNS];

CIP_Connection* GetExclusiveOwnerConnection(CIP_Connection* connection_object, CipUint* extended_error);

CIP_Connection* GetInputOnlyConnection(CIP_Connection* connection_object, CipUint* extended_error);

CIP_Connection* GetListenOnlyConnection(CIP_Connection* connection_object, CipUint* extended_error);

void ConfigureExclusiveOwnerConnectionPoint(unsigned int connection_number,
    unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS > connection_number)
    {
        g_exlusive_owner_connections[connection_number].output_assembly = output_assembly;
        g_exlusive_owner_connections[connection_number].input_assembly = input_assembly;
        g_exlusive_owner_connections[connection_number].config_assembly = config_assembly;
    }
}

void ConfigureInputOnlyConnectionPoint(unsigned int connection_number,
    unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_INPUT_ONLY_CONNS > connection_number)
    {
        g_input_only_connections[connection_number].output_assembly = output_assembly;
        g_input_only_connections[connection_number].input_assembly = input_assembly;
        g_input_only_connections[connection_number].config_assembly = config_assembly;
    }
}

void ConfigureListenOnlyConnectionPoint(unsigned int connection_number,
    unsigned int output_assembly,
    unsigned int input_assembly,
    unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_LISTEN_ONLY_CONNS > connection_number)
    {
        g_listen_only_connections[connection_number].output_assembly = output_assembly;
        g_listen_only_connections[connection_number].input_assembly = input_assembly;
        g_listen_only_connections[connection_number].config_assembly = config_assembly;
    }
}

CIP_Connection* GetIoConnectionForConnectionData( CIP_Connection* connection_object, CipUint* extended_error)
{
    CIP_Connection* io_connection = NULL;
    *extended_error = 0;

    io_connection = GetExclusiveOwnerConnection(connection_object, extended_error);
    if (NULL == io_connection)
    {
        if (0 == *extended_error)
        {
            /* we found no connection and don't have an error so try input only next */
            io_connection = GetInputOnlyConnection(connection_object, extended_error);
            if (NULL == io_connection)
            {
                if (0 == *extended_error)
                {
                    /* we found no connection and don't have an error so try listen only next */
                    io_connection = GetListenOnlyConnection(connection_object, extended_error);
                    if ((NULL == io_connection) && (0 == *extended_error))
                    {
                        /* no application connection type was found that suits the given data */
                        /* TODO check error code VS */
                        *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                    }
                    else
                    {
                        connection_object->instance_type = kConnectionTypeIoListenOnly;
                    }
                }
            }
            else
            {
                connection_object->instance_type = kConnectionTypeIoInputOnly;
            }
        }
    }
    else
    {
        connection_object->instance_type = kConnectionTypeIoExclusiveOwner;
    }

    if (NULL != io_connection)
    {
        CIP_Connection::CopyConnectionData(io_connection, connection_object);
    }

    return io_connection;
}

CIP_Connection* GetExclusiveOwnerConnection(CIP_Connection* connection_object, CipUint* extended_error)
{
    CIP_Connection* exclusive_owner_connection = NULL;
    int i;

    for (i = 0; i < OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS; i++) {
        if ((g_exlusive_owner_connections[i].output_assembly
                == connection_object->connection_path.connection_point[0])
            && (g_exlusive_owner_connections[i].input_assembly
                   == connection_object->connection_path.connection_point[1])
            && (g_exlusive_owner_connections[i].config_assembly
                   == connection_object->connection_path.connection_point[2])) {

            /* check if on other connection point with the same output assembly is currently connected */
            if (NULL
                != CIP_Connection::GetConnectedOutputAssembly(
                       connection_object->connection_path.connection_point[0])) {
                *extended_error = kConnectionManagerStatusCodeErrorOwnershipConflict;
                break;
            }
            exclusive_owner_connection = (g_exlusive_owner_connections[i]
                                               .connection_data);
            break;
        }
    }
    return exclusive_owner_connection;
}

CIP_Connection* GetInputOnlyConnection(CIP_Connection* connection_object,
    CipUint* extended_error)
{
    CIP_Connection* input_only_connection = NULL;
    int i, j;

    for (i = 0; i < OPENER_CIP_NUM_INPUT_ONLY_CONNS; i++) {
        if (g_input_only_connections[i].output_assembly
            == connection_object->connection_path.connection_point[0]) { /* we have the same output assembly */
            if (g_input_only_connections[i].input_assembly
                != connection_object->connection_path.connection_point[1]) {
                *extended_error = kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_input_only_connections[i].config_assembly
                != connection_object->connection_path.connection_point[2]) {
                *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH; j++) {
                if (kConnectionStateNonExistent
                    == g_input_only_connections[i].connection_data[j]->state) {
                    return (g_input_only_connections[i].connection_data[j]);
                }
            }
            *extended_error = kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return input_only_connection;
}

CIP_Connection* GetListenOnlyConnection(CIP_Connection* connection_object,
    CipUint* extended_error)
{
    CIP_Connection* listen_only_connection = NULL;
    int i, j;

    if (kRoutingTypeMulticastConnection != (connection_object->t_to_o_network_connection_parameter & kRoutingTypeMulticastConnection))
    {
        /* a listen only connection has to be a multicast connection. */
        *extended_error = kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened; /* maybe not the best error message however there is no suitable definition in the cip spec */
        return NULL;
    }

    for (i = 0; i < OPENER_CIP_NUM_LISTEN_ONLY_CONNS; i++)
    {
        if (g_listen_only_connections[i].output_assembly == connection_object->connection_path.connection_point[0])
        { /* we have the same output assembly */
            if (g_listen_only_connections[i].input_assembly != connection_object->connection_path.connection_point[1])
            {
                *extended_error = kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_listen_only_connections[i].config_assembly != connection_object->connection_path.connection_point[2])
            {
                *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            if (NULL == GetExistingProducerMulticastConnection(connection_object->connection_path.connection_point[1]))
            {
                *extended_error = kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH; j++)
            {
                if (kConnectionStateNonExistent
                    == g_listen_only_connections[i].connection_data[j]->state)
                {
                    return g_listen_only_connections[i].connection_data[j];
                }
            }
            *extended_error = kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return listen_only_connection;
}

CIP_Connection* GetExistingProducerMulticastConnection(CipUdint input_point)
{
    CIP_Connection* producer_multicast_connection;

    for(int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {
        producer_multicast_connection = CIP_Connection::active_connections_set[i];

        if ((kConnectionTypeIoExclusiveOwner == producer_multicast_connection->instance_type)
            || (kConnectionTypeIoInputOnly   == producer_multicast_connection->instance_type)) 
        {
            if ((input_point == producer_multicast_connection->connection_path.connection_point[1])
                && (kRoutingTypeMulticastConnection == (producer_multicast_connection->t_to_o_network_connection_parameter & kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket!= producer_multicast_connection->socket[kUdpCommuncationDirectionProducing]))
            {
                /* we have a connection that produces the same input assembly,
         * is a multicast producer and manages the connection.
         */
                break;
            }
        }
    }
    return producer_multicast_connection;
}

CIP_Connection* GetNextNonControlMasterConnection(CipUdint input_point)
{
    CIP_Connection* next_non_control_master_connection;

     for(int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {
        next_non_control_master_connection = CIP_Connection::active_connections_set[i];

        if ((kConnectionTypeIoExclusiveOwner == next_non_control_master_connection->instance_type)
            || (kConnectionTypeIoInputOnly == next_non_control_master_connection->instance_type))
        {
            if ((input_point == next_non_control_master_connection->connection_path.connection_point[1])
                && (kRoutingTypeMulticastConnection == (next_non_control_master_connection->t_to_o_network_connection_parameter
                              & kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket == next_non_control_master_connection->socket[kUdpCommuncationDirectionProducing])) {
                /* we have a connection that produces the same input assembly,
         * is a multicast producer and does not manages the connection.
         */
                break;
            }
        }
    }
    return next_non_control_master_connection;
}

void CloseAllConnectionsForInputWithSameType(CipUdint input_point,
    ConnectionType instance_type)
{
    CIP_Connection* connection;
    CIP_Connection* connection_to_delete;

    for(int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {
        connection = CIP_Connection::active_connections_set[i];

        if ((instance_type == connection->instance_type) && (input_point == connection->connection_path.connection_point[1]))
        {

            CheckIoConnectionEvent(
                connection->connection_path.connection_point[0],
                connection->connection_path.connection_point[1],
                kIoConnectionEventClosed);

            /* FIXME check if this is ok */
            //connection_to_delete->connection_close_function(connection_to_delete);
            CIP_Connection::CloseConnection(connection); /*will remove the connection from the active connection list */

            CIP_Connection::active_connections_set.erase(i);
        } 
    }
}

void CloseAllConnections(void)
{
    CIP_Connection* connection;
    for(int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {
        connection = CIP_Connection::active_connections_set[i];
        /*FIXME check if m_pfCloseFunc would be suitable*/
        CIP_Connection::CloseConnection(connection);
        /* Close connection will remove the connection from the list therefore we
     * need to get again the start until there is no connection left
     */
    }
}

CipBool ConnectionWithSameConfigPointExists(CipUdint config_point)
{
    CIP_Connection* connection;
    for(int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {
        connection = CIP_Connection::active_connections_set[i];

        if (config_point == connection->connection_path.connection_point[2]) {
            break;
        }
    }
    return (NULL != connection);
}

void InitializeIoConnectionData(void)
{
    memset(g_exlusive_owner_connections, 0, OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS * sizeof(ExclusiveOwnerConnection));
    memset(g_input_only_connections, 0, OPENER_CIP_NUM_INPUT_ONLY_CONNS * sizeof(InputOnlyConnection));
    memset(g_listen_only_connections, 0,  OPENER_CIP_NUM_LISTEN_ONLY_CONNS * sizeof(ListenOnlyConnection));
}
