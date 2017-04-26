/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include "CIP_AppConnType.hpp"

//Static variables
CIP_AppConnType::ExclusiveOwnerConnection * CIP_AppConnType::g_exlusive_owner_connections;

CIP_AppConnType::InputOnlyConnection * CIP_AppConnType::g_input_only_connections;

CIP_AppConnType::ListenOnlyConnection * CIP_AppConnType::g_listen_only_connections;

//Methods

void CIP_AppConnType::ConfigureExclusiveOwnerConnectionPoint(unsigned int connection_number,
    unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS > connection_number)
    {
        g_exlusive_owner_connections[connection_number].output_assembly = output_assembly;
        g_exlusive_owner_connections[connection_number].input_assembly = input_assembly;
        g_exlusive_owner_connections[connection_number].config_assembly = config_assembly;
    }
}

void CIP_AppConnType::ConfigureInputOnlyConnectionPoint(unsigned int connection_number, unsigned int output_assembly, unsigned int input_assembly, unsigned int config_assembly)
{
    if (OPENER_CIP_NUM_INPUT_ONLY_CONNS > connection_number)
    {
        g_input_only_connections[connection_number].output_assembly = output_assembly;
        g_input_only_connections[connection_number].input_assembly = input_assembly;
        g_input_only_connections[connection_number].config_assembly = config_assembly;
    }
}

void CIP_AppConnType::ConfigureListenOnlyConnectionPoint(unsigned int connection_number,
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

const  CIP_Connection* CIP_AppConnType::GetIoConnectionForConnectionData(const CIP_ConnectionManager* connection_manager, CipUint* extended_error)
{
    const CIP_Connection* io_connection = nullptr;
    *extended_error = 0;

    io_connection = GetExclusiveOwnerConnection(connection_manager, extended_error);
    if (nullptr == io_connection)
    {
        if (0 == *extended_error)
        {
            /* we found no connection and don't have an error so try input only next */
            io_connection = GetInputOnlyConnection(connection_manager, extended_error);
            if (nullptr == io_connection)
            {
                if (0 == *extended_error)
                {
                    /* we found no connection and don't have an error so try listen only next */
                    io_connection = GetListenOnlyConnection(connection_manager, extended_error);
                    if ((nullptr == io_connection) && (0 == *extended_error))
                    {
                        /* no application connection type was found that suits the given data */
                        /* TODO check error code VS */
                        *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeInconsistentApplicationPathCombo;
                    }
                    else
                    {
                        ((CIP_Connection*)connection_manager)->Instance_type = CIP_Connection::kConnectionTypeIo;//CIP_AppConnType::kAppConnTypeIoListenOnly;
                    }
                }
            }
            else
            {
                ((CIP_Connection*)connection_manager)->Instance_type = CIP_Connection::kConnectionTypeIo; //CIP_AppConnType::kAppConnTypeIoInputOnly;
            }
        }
    }
    else
    {
        ((CIP_Connection*)connection_manager)->Instance_type = CIP_Connection::kConnectionTypeIo; //CIP_AppConnType::kAppConnTypeIoExclusiveOwner;
    }

    if (nullptr != io_connection)
    {
        //todo: fix CIP_ConnectionManager::CopyConnectionData((CIP_Connection*)io_connection, connection_manager->producing_instance);
    }

    return io_connection;
}

const  CIP_Connection* CIP_AppConnType::GetExclusiveOwnerConnection(const CIP_ConnectionManager* connection_manager, CipUint* extended_error)
{
    CIP_Connection* exclusive_owner_connection = nullptr;
    int i;

    for (i = 0; i < OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS; i++)
    {
        if ((g_exlusive_owner_connections[i].output_assembly == connection_manager->connection_path.connection_point[0])
            && (g_exlusive_owner_connections[i].input_assembly == connection_manager->connection_path.connection_point[1])
            && (g_exlusive_owner_connections[i].config_assembly == connection_manager->connection_path.connection_point[2]))
        {

            /* check if on other connection point with the same output assembly is currently connected */
            //todo: fix if (nullptr != CIP_ConnectionManager::GetConnectedOutputAssembly(connection_manager->connection_path.connection_point[0]))
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeErrorOwnershipConflict;
                break;
            }
            exclusive_owner_connection = (g_exlusive_owner_connections[i].connection_data);
            break;
        }
    }
    return exclusive_owner_connection;
}

const CIP_Connection* CIP_AppConnType::GetInputOnlyConnection(const CIP_ConnectionManager* connection_manager, CipUint* extended_error)
{
    CIP_Connection* input_only_connection = nullptr;
    int i, j;

    for (i = 0; i < OPENER_CIP_NUM_INPUT_ONLY_CONNS; i++)
    {
        if (g_input_only_connections[i].output_assembly == connection_manager->connection_path.connection_point[0])
        { /* we have the same output assembly */
            if (g_input_only_connections[i].input_assembly != connection_manager->connection_path.connection_point[1])
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_input_only_connections[i].config_assembly!= connection_manager->connection_path.connection_point[2])
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH; j++)
            {
                if (CIP_Connection::kConnectionStateNonExistent == g_input_only_connections[i].connection_data[j].State)
                {
                    return (&g_input_only_connections[i].connection_data[j]);
                }
            }
            *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return input_only_connection;
}

const CIP_Connection* CIP_AppConnType::GetListenOnlyConnection(const CIP_ConnectionManager* connection_manager, CipUint* extended_error)
{
    CIP_Connection* listen_only_connection = nullptr;
    int i, j;

    if (CIP_ConnectionManager::kRoutingTypeMulticastConnection != connection_manager->t_to_o_network_connection_parameter)
    {
        /* a listen only connection has to be a multicast connection. */
        *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeNonListenOnlyConnectionNotOpened; /* maybe not the best error message however there is no suitable definition in the cip spec */
        return nullptr;
    }

    for (i = 0; i < OPENER_CIP_NUM_LISTEN_ONLY_CONNS; i++)
    {
        if (g_listen_only_connections[i].output_assembly == connection_manager->connection_path.connection_point[0])
        { /* we have the same output assembly */
            if (g_listen_only_connections[i].input_assembly != connection_manager->connection_path.connection_point[1])
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeInvalidProducingApplicationPath;
                break;
            }
            if (g_listen_only_connections[i].config_assembly != connection_manager->connection_path.connection_point[2])
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            if (nullptr == GetExistingProducerMulticastConnection(connection_manager->connection_path.connection_point[1]))
            {
                *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeNonListenOnlyConnectionNotOpened;
                break;
            }

            for (j = 0; j < OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH; j++)
            {
                if (CIP_Connection::kConnectionStateNonExistent == g_listen_only_connections[i].connection_data[j].State)
                {
                    return &g_listen_only_connections[i].connection_data[j];
                }
            }
            *extended_error = CIP_ConnectionManager::kConnMgrStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }
    return listen_only_connection;
}

const CIP_Connection* CIP_AppConnType::GetExistingProducerMulticastConnection(CipUdint input_point)
{
    const CIP_ConnectionManager* producer_multicast_connection_manager;

    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size(); i++)
    {
        producer_multicast_connection_manager = CIP_ConnectionManager::active_connections_set->at(i);

        if (CIP_Connection::kConnectionTypeIo == producer_multicast_connection_manager->producing_instance->Instance_type) //check against original
        {
            if ((input_point == producer_multicast_connection_manager->connection_path.connection_point[1])
                && (CIP_ConnectionManager::kRoutingTypeMulticastConnection == (producer_multicast_connection_manager->t_to_o_network_connection_parameter & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket!= producer_multicast_connection_manager->producing_instance->netConn->GetSocketHandle()))//todo:kUdpCommuncationDirectionProducing)))
            {
                /* we have a connection that produces the same input assembly,
         * is a multicast producer and manages the connection.
         */
                break;
            }
        }
    }
    return producer_multicast_connection_manager->producing_instance;
}

const CIP_Connection* CIP_AppConnType::GetNextNonControlMasterConnection(CipUdint input_point)
{
    CIP_ConnectionManager* next_non_control_master_connection_manager;

     for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size(); i++)
    {
        next_non_control_master_connection_manager = (CIP_ConnectionManager*)CIP_ConnectionManager::active_connections_set->at(i);

        if (CIP_Connection::kConnectionTypeIo == next_non_control_master_connection_manager->producing_instance->Instance_type)
        //todo: check against original if ((CIP_Connection::kConnectionTypeIo; //CIP_AppConnType::kAppConnTypeIoExclusiveOwner == next_non_control_master_connection->Instance_type)
        //    || (CIP_Connection::kConnectionTypeIo; //CIP_AppConnType::kAppConnTypeIoInputOnly == next_non_control_master_connection->Instance_type))
        {
            if ((input_point == next_non_control_master_connection_manager->connection_path.connection_point[1])
                && (CIP_ConnectionManager::kRoutingTypeMulticastConnection == (next_non_control_master_connection_manager->t_to_o_network_connection_parameter
                              & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
                && (kEipInvalidSocket == next_non_control_master_connection_manager->producing_instance->netConn->GetSocketHandle()))//todo:kUdpCommuncationDirectionProducing)))
            {
                /* we have a connection that produces the same input assembly,
         * is a multicast producer and does not manages the connection.
         */
                break;
            }
        }
    }
    return next_non_control_master_connection_manager->producing_instance;
}

void CIP_AppConnType::CloseAllConnectionsForInputWithSameType(CipUdint input_point, CIP_Connection::ConnectionType_e Instance_type)
{
    const CIP_ConnectionManager* connection_manager;
    const CIP_Connection* connection_to_delete;

    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size(); i++)
    {
        connection_manager = CIP_ConnectionManager::active_connections_set->at(i);

        //check instance against original
        if ((Instance_type == connection_manager->producing_instance->Instance_type) && (input_point == connection_manager->connection_path.connection_point[1]))
        {

            //TODO:CheckIoConnectionEvent(connection_manager->connection_path.connection_point[0], connection_manager->connection_path.connection_point[1], kIoConnectionEventClosed);

            /* FIXME check if this is ok */
            //connection_to_delete->connection_close_function(connection_to_delete);
            //todo:fix CIP_ConnectionManager::CloseConnection ((CIP_ConnectionManager*)connection_manager); /*will remove the connection from the active connection list */
        }
    }
}

void CIP_AppConnType::CloseAllConnections(void)
{
    CIP_ConnectionManager* connection_manager;
    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size(); i++)
    {
        connection_manager = (CIP_ConnectionManager*)CIP_ConnectionManager::active_connections_set->at(i);
        /*FIXME check if m_pfCloseFunc would be suitable*/
        //todo: fix CIP_ConnectionManager::CloseConnection(connection_manager);
        /* Close connection will remove the connection from the list therefore we
     * need to get again the start until there is no connection left
     */
    }
}

CipBool CIP_AppConnType::ConnectionWithSameConfigPointExists(CipUdint config_point)
{
    CIP_ConnectionManager* connection_manager;
    for(unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size(); i++)
    {
        connection_manager = (CIP_ConnectionManager*)CIP_ConnectionManager::active_connections_set->at(i);

        if (config_point == connection_manager->connection_path.connection_point[2]) {
            break;
        }
    }
    return (CipBool) (nullptr != connection_manager);
}

void CIP_AppConnType::InitializeIoConnectionData(void)
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
