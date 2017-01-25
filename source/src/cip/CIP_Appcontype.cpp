/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include "CIP_Appcontype.hpp"

//Static variables
CIP_Appcontype::ExclusiveOwnerConnection * CIP_Appcontype::g_exlusive_owner_connections;

CIP_Appcontype::InputOnlyConnection * CIP_Appcontype::g_input_only_connections;

CIP_Appcontype::ListenOnlyConnection * CIP_Appcontype::g_listen_only_connections;

//Methods

void CIP_Appcontype::ConfigureExclusiveOwnerConnectionPoint(unsigned int connection_number,
    unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS > connection_number)
    {
        g_exlusive_owner_connections[connection_number].output_assembly = output_assembly;
        g_exlusive_owner_connections[connection_number].input_assembly = input_assembly;
        g_exlusive_owner_connections[connection_number].config_assembly = config_assembly;
    }
}

void CIP_Appcontype::ConfigureInputOnlyConnectionPoint(unsigned int connection_number, unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_INPUT_ONLY_CONNS > connection_number)
    {
        g_input_only_connections[connection_number].output_assembly = output_assembly;
        g_input_only_connections[connection_number].input_assembly = input_assembly;
        g_input_only_connections[connection_number].config_assembly = config_assembly;
    }
}

void CIP_Appcontype::ConfigureListenOnlyConnectionPoint(unsigned int connection_number,
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

const  CIP_ConnectionManager* CIP_Appcontype::GetIoConnectionForConnectionData(const CIP_ConnectionManager* connection_object, CipUint* extended_error)
{
    const CIP_ConnectionManager* io_connection = NULL;
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
                        *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                    }
                    else
                    {
                        ((CIP_ConnectionManager*)connection_object)->instance_type = CIP_ConnectionManager::kConnectionTypeIoListenOnly;
                    }
                }
            }
            else
            {
                ((CIP_ConnectionManager*)connection_object)->instance_type = CIP_ConnectionManager::kConnectionTypeIoInputOnly;
            }
        }
    }
    else
    {
        ((CIP_ConnectionManager*)connection_object)->instance_type = CIP_ConnectionManager::kConnectionTypeIoExclusiveOwner;
    }

    if (NULL != io_connection)
    {
        CIP_ConnectionManager::CopyConnectionData(io_connection, connection_object);
    }

    return io_connection;
}

const  CIP_ConnectionManager* CIP_Appcontype::GetExclusiveOwnerConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error)
{
    CIP_ConnectionManager* exclusive_owner_connection = NULL;
    int i;

    for (i = 0; i < OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS; i++)
    {
        if ((g_exlusive_owner_connections[i].output_assembly == connection_object->connection_path.connection_point[0])
            && (g_exlusive_owner_connections[i].input_assembly == connection_object->connection_path.connection_point[1])
            && (g_exlusive_owner_connections[i].config_assembly == connection_object->connection_path.connection_point[2]))
        {

            /* check if on other connection point with the same output assembly is currently connected */
            if (NULL != CIP_ConnectionManager::GetConnectedOutputAssembly(connection_object->connection_path.connection_point[0]))
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorOwnershipConflict;
                break;
            }
            exclusive_owner_connection = (g_exlusive_owner_connections[i]
                                               .connection_data);
            break;
        }
    }
    return exclusive_owner_connection;
}

const CIP_ConnectionManager* CIP_Appcontype::GetInputOnlyConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error)
{
    CIP_ConnectionManager* input_only_connection = NULL;
    int i, j;

    for (i = 0; i < OPENER_CIP_NUM_INPUT_ONLY_CONNS; i++)
    {
        if (g_input_only_connections[i].output_assembly == connection_object->connection_path.connection_point[0])
        { /* we have the same output assembly */
            if (g_input_only_connections[i].input_assembly != connection_object->connection_path.connection_point[1])
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_input_only_connections[i].config_assembly!= connection_object->connection_path.connection_point[2])
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH; j++)
            {
                if (CIP_ConnectionManager::kConnectionStateNonExistent == g_input_only_connections[i].connection_data[j].state)
                {
                    return (&g_input_only_connections[i].connection_data[j]);
                }
            }
            *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return input_only_connection;
}

const CIP_ConnectionManager* CIP_Appcontype::GetListenOnlyConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error)
{
    CIP_ConnectionManager* listen_only_connection = NULL;
    int i, j;

    if (CIP_ConnectionManager::kRoutingTypeMulticastConnection != (connection_object->t_to_o_network_connection_parameter & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
    {
        /* a listen only connection has to be a multicast connection. */
        *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened; /* maybe not the best error message however there is no suitable definition in the cip spec */
        return NULL;
    }

    for (i = 0; i < OPENER_CIP_NUM_LISTEN_ONLY_CONNS; i++)
    {
        if (g_listen_only_connections[i].output_assembly == connection_object->connection_path.connection_point[0])
        { /* we have the same output assembly */
            if (g_listen_only_connections[i].input_assembly != connection_object->connection_path.connection_point[1])
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_listen_only_connections[i].config_assembly != connection_object->connection_path.connection_point[2])
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            if (NULL == GetExistingProducerMulticastConnection(connection_object->connection_path.connection_point[1]))
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH; j++)
            {
                if (CIP_ConnectionManager::kConnectionStateNonExistent == g_listen_only_connections[i].connection_data[j].state)
                {
                    return &g_listen_only_connections[i].connection_data[j];
                }
            }
            *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return listen_only_connection;
}

const CIP_ConnectionManager* CIP_Appcontype::GetExistingProducerMulticastConnection(CipUdint input_point)
{
    const CIP_ConnectionManager* producer_multicast_connection;

    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set.size(); i++)
    {
        producer_multicast_connection = CIP_ConnectionManager::active_connections_set[i];

        if ((CIP_ConnectionManager::kConnectionTypeIoExclusiveOwner == producer_multicast_connection->instance_type) || (CIP_ConnectionManager::kConnectionTypeIoInputOnly   == producer_multicast_connection->instance_type))
        {
            if ((input_point == producer_multicast_connection->connection_path.connection_point[1])
                && (CIP_ConnectionManager::kRoutingTypeMulticastConnection == (producer_multicast_connection->t_to_o_network_connection_parameter & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket!= producer_multicast_connection->netConn->GetSocketHandle()))//todo:kUdpCommuncationDirectionProducing)))
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

const CIP_ConnectionManager* CIP_Appcontype::GetNextNonControlMasterConnection(CipUdint input_point)
{
    CIP_ConnectionManager* next_non_control_master_connection;

     for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set.size(); i++)
    {
        next_non_control_master_connection = (CIP_ConnectionManager*)CIP_ConnectionManager::active_connections_set[i];

        if ((CIP_ConnectionManager::kConnectionTypeIoExclusiveOwner == next_non_control_master_connection->instance_type)
            || (CIP_ConnectionManager::kConnectionTypeIoInputOnly == next_non_control_master_connection->instance_type))
        {
            if ((input_point == next_non_control_master_connection->connection_path.connection_point[1])
                && (CIP_ConnectionManager::kRoutingTypeMulticastConnection == (next_non_control_master_connection->t_to_o_network_connection_parameter
                              & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket == next_non_control_master_connection->netConn->GetSocketHandle()))//todo:kUdpCommuncationDirectionProducing)))
            {
                /* we have a connection that produces the same input assembly,
         * is a multicast producer and does not manages the connection.
         */
                break;
            }
        }
    }
    return next_non_control_master_connection;
}

void CIP_Appcontype::CloseAllConnectionsForInputWithSameType(CipUdint input_point, CIP_ConnectionManager::ConnectionType instance_type)
{
    const CIP_ConnectionManager* connection;
    const CIP_ConnectionManager* connection_to_delete;

    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set.size(); i++)
    {
        connection = CIP_ConnectionManager::active_connections_set[i];

        if ((instance_type == connection->instance_type) && (input_point == connection->connection_path.connection_point[1]))
        {

            //TODO:CheckIoConnectionEvent(connection->connection_path.connection_point[0], connection->connection_path.connection_point[1], kIoConnectionEventClosed);

            /* FIXME check if this is ok */
            //connection_to_delete->connection_close_function(connection_to_delete);
            ((CIP_ConnectionManager*)connection)->CloseConnection (); /*will remove the connection from the active connection list */
        }
    }
}

void CIP_Appcontype::CloseAllConnections(void)
{
    const  CIP_ConnectionManager* connection;
    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set.size(); i++)
    {
        connection = CIP_ConnectionManager::active_connections_set[i];
        /*FIXME check if m_pfCloseFunc would be suitable*/
        ((CIP_ConnectionManager*)connection)->CloseConnection();
        /* Close connection will remove the connection from the list therefore we
     * need to get again the start until there is no connection left
     */
    }
}

CipBool CIP_Appcontype::ConnectionWithSameConfigPointExists(CipUdint config_point)
{
    CIP_ConnectionManager* connection;
    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set.size(); i++)
    {
        connection = (CIP_ConnectionManager*)CIP_ConnectionManager::active_connections_set[i];

        if (config_point == connection->connection_path.connection_point[2]) {
            break;
        }
    }
    return (CipBool) (NULL != connection);
}

void CIP_Appcontype::InitializeIoConnectionData(void)
{
    g_exlusive_owner_connections = new ExclusiveOwnerConnection[OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS]();


    g_input_only_connections = new InputOnlyConnection[OPENER_CIP_NUM_INPUT_ONLY_CONNS]();
    /*for (int i = 0; i < OPENER_CIP_NUM_INPUT_ONLY_CONNS; i++)
    {
        g_input_only_connections[i].connection_data = new CIP_ConnectionManager[OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH]();
    }*/

    g_listen_only_connections = new ListenOnlyConnection[OPENER_CIP_NUM_LISTEN_ONLY_CONNS]();
    /*for (int i = 0; i < OPENER_CIP_NUM_LISTEN_ONLY_CONNS; i++)
    {
        g_listen_only_connections[i].connection_data = new CIP_ConnectionManager[OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH]();
    }*/
}
