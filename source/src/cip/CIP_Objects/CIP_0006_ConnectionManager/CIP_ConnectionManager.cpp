/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include "CIP_ConnectionManager.hpp"
#include <cstring>
#include "../CIP_0001_Identity/CIP_Identity.hpp"
#include "../../connection/network/NET_Endianconv.hpp"
#include "cip/CIP_AppConnType.hpp"
#include "network/NET_NetworkHandler.hpp"
#include "cip/CIP_Objects/CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"

#define CIP_CONN_TYPE_MASK 0x6000 /**< Bit mask filter on bit 13 & 14 */

const int g_kForwardOpenHeaderLength = 36; /**< the length in bytes of the forward open command specific data till the start of the connection path (including con path size)*/

/** @brief Compares the logical path on equality */
#define EQLOGICALPATH(x, y) (((x)&0xfc) == (y))

static const int g_kNumberOfConnectableObjects = 2 + OPENER_CIP_NUM_APPLICATION_SPECIFIC_CONNECTABLE_OBJECTS;


std::map<CipUdint, const CIP_ConnectionManager *> *CIP_ConnectionManager::active_connections_set;
std::map<CipUdint, ConnectionManagementHandling> *CIP_ConnectionManager::g_astConnMgmList;
CIP_ConnectionManager * CIP_ConnectionManager::g_dummy_connection_object;
CipUdint CIP_ConnectionManager::g_incarnation_id;

/*TEMPORARY EXPLICIT CONN STUFF*/
CipStatus EstablishExplicitConnection()
{
   CipStatus eip_status;
    eip_status.status = kCipStatusError;

    CipStatus connCreateStatus = CIP_Connection::Create();
    CIP_Connection* connection = (CIP_Connection*)( connCreateStatus.status==kCipStatusOk ?
                                          CIP_Connection::GetInstance (connCreateStatus.extended_status) : nullptr);
    if (nullptr == connection)
    {
        eip_status.status = kCipErrorConnectionFailure;
        eip_status.extended_status = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorNoMoreConnectionsAvailable;
    }
    else
    {
        CIP_ConnectionManager::CopyConnectionData(connection, this);

        produced_connection_id_buffer = connection->CIP_produced_connection_id;
        CIP_ConnectionManager::GeneralConnectionConfiguration (connection);
        connection->CIP_produced_connection_id = produced_connection_id_buffer;
        connection->Instance_type = CIP_Connection::kConnectionTypeExplicit;
        connection->netConn->SetSocketHandle (kEipInvalidSocket);
        /* set the connection call backs */
        connection->connection_close_function = CIP_ConnectionManager::RemoveFromActiveConnections;
        /* explicit connection have to be closed on time out*/
        connection->connection_timeout_function = CIP_ConnectionManager::RemoveFromActiveConnections;

        CIP_ConnectionManager::AddNewActiveConnection(connection);
    }
    return eip_status;
}

/*TEMPORARY IO CONN STUFF*/

//Includes

const int CIP_ConnectionManager::kOpENerEipIoUdpPort = 0x08AE;
CipUsint* CIP_ConnectionManager::g_config_data_buffer; /**< buffers for the config data coming with a forward open request. */
unsigned int CIP_ConnectionManager::g_config_data_length;
CipUdint CIP_ConnectionManager::g_run_idle_state; /**< buffer for holding the run idle information. */

//Methods
void CIP_ConnectionManager::InitializeIOConnectionData()
{
    //init variables
    g_config_data_buffer = nullptr;
    g_config_data_length = 0;
}
CipStatus CIP_ConnectionManager::EstablishIoConnection (CIP_Connection * connection_instance, CipUint *extended_error)
{
    int originator_to_target_connection_type, target_to_originator_connection_type;
    CipStatus eip_status = kCipStatusOk;
    CIP_Attribute* attribute;

    // currently we allow I/O connections only to assembly objects
    // we don't need to check for zero as this is handled in the connection path parsing
    const CIP_Assembly* assembly_class = CIP_Assembly::GetClass();
    CIP_ConnectionManager* instance = (CIP_ConnectionManager*)CIP_ConnectionManager::GetConnectionManagerObject (connection_instance->id);

    CIP_AppConnType::GetIoConnectionForConnectionData(instance, extended_error);

    //if (nullptr == this)
    {
        //return (CipStatus) kCipErrorConnectionFailure;
    }

    // TODO: add check for transport type trigger

    if (CIP_Connection::kConnectionTriggerProductionTriggerCyclic
        != connection_instance->TransportClass_trigger.bitfield_u.production_trigger)
    {
        if (256 == instance->production_inhibit_time)
        {
            //there was no PIT segment in the connection path set PIT to one fourth of RPI
            instance->production_inhibit_time = (CipUint) ((CipUint)(instance->t_to_o_requested_packet_interval) / 4000);
        }
        else
        {
            // if production inhibit time has been provided it needs to be smaller than the RPI
            if (instance->production_inhibit_time > ((CipUint)((instance->t_to_o_requested_packet_interval) / 1000)))
            {
                // see section C-1.4.3.3
                *extended_error = 0x111; //< RPI not supported. Extended Error code deprecated
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }
    }
    // set the connection call backs
    //connection_close_function = CloseIoConnection;
    //connection_timeout_function = HandleIoConnectionTimeOut;
    //connection_send_data_function = SendConnectedData;
    //connection_receive_data_function = HandleReceivedIoConnectionData;

    CIP_ConnectionManager::GeneralConnectionConfiguration (instance);
    originator_to_target_connection_type = (instance->o_to_t_network_connection_parameter & 0x6000) >> 13;
    target_to_originator_connection_type = (instance->t_to_o_network_connection_parameter & 0x6000) >> 13;

    if ((originator_to_target_connection_type == 0) && (target_to_originator_connection_type == 0))
    {
        // this indicates an re-configuration of the connection currently not supported and
        // we should not come here as this is handled in the forwardopen function

    }
    else
    {
        int producing_index = 0;
        int data_size;
        int diff_size;
        int is_heartbeat;

        if ((originator_to_target_connection_type != 0) && (target_to_originator_connection_type != 0))
        {
            // we have a producing and consuming connection
            producing_index = 1;
        }

        instance->consuming_instance = 0;
        connection_instance->Consumed_connection_path_length = 0;
        instance->producing_instance = 0;
        connection_instance->Produced_connection_path_length = 0;

        if (originator_to_target_connection_type != 0)
        {
            //setup consumer side
            if (nullptr == (instance = (CIP_ConnectionManager*) CIP_Assembly::GetInstance (instance->connection_path.connection_point[0])))
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInvalidConsumingApllicationPath;
                return (CipStatus) kCipErrorConnectionFailure;
            }
            // consuming Connection Point is present
            instance->consuming_instance = connection_instance;

            connection_instance->Consumed_connection_path_length = 6;
            connection_instance->Consumed_connection_path.path_size = 6;
            connection_instance->Consumed_connection_path.class_id = (CipUint) instance->connection_path.class_id;
            connection_instance->Consumed_connection_path.instance_number = (CipUint) instance->connection_path.connection_point[0];
            connection_instance->Consumed_connection_path.attribute_number = 3;

            attribute = instance->GetCipAttribute (3);
            OPENER_ASSERT(attribute != nullptr);
            // an assembly object should always have an attribute 3
            data_size = connection_instance->Consumed_connection_size;
            diff_size = 0;
            is_heartbeat = (((CipByteArray*)attribute->getData ())->length == 0);

            if (connection_instance->TransportClass_trigger.bitfield_u.transport_class == kConnectionTriggerTransportClass1)//transport_type_class_trigger & 0x0F) == 1)
            {
                // class 1 connection
                data_size -= 2; // remove 16-bit sequence count length
                diff_size += 2;
            }

            if ((kOpENerConsumedDataHasRunIdleHeader) && (data_size > 0) && (!is_heartbeat))
            {
                // we only have an run idle header if it is not an heartbeat connection
                data_size -= 4; // remove the 4 bytes needed for run/idle header
                diff_size += 4;
            }

            if (((CipByteArray*)attribute->getData())->length != data_size)
            {
                /*wrong connection size */
                instance->correct_originator_to_target_size = (CipUint) (((CipByteArray*)attribute->getData())->length + diff_size);
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize;
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        if (target_to_originator_connection_type != 0)
        {
            //setup producer side
            if (0 == (instance = (CIP_ConnectionManager*) CIP_Assembly::GetInstance(instance->connection_path.connection_point[producing_index])))
            {
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                return (CipStatus) kCipErrorConnectionFailure;
            }
            instance->producing_instance = connection_instance;

            connection_instance->Produced_connection_path_length = 6;
            connection_instance->Produced_connection_path.path_size = 6;
            connection_instance->Produced_connection_path.class_id = (CipUint) instance->connection_path.class_id;
            connection_instance->Produced_connection_path.instance_number = (CipUint) instance->connection_path.connection_point[producing_index];
            connection_instance->Produced_connection_path.attribute_number = 3;

            attribute = ((CIP_Object*)instance)->GetCipAttribute (3);
            OPENER_ASSERT(attribute != nullptr);
            // an assembly object should always have an attribute 3
            data_size = connection_instance->Produced_connection_size;
            diff_size = 0;
            is_heartbeat = (((CipByteArray*)attribute->getData())->length == 0);

            if (connection_instance->TransportClass_trigger.bitfield_u.transport_class == kConnectionTriggerTransportClass1)
            {
                // class 1 connection
                data_size -= 2; //remove 16-bit sequence count length
                diff_size += 2;
            }

            if ((kOpENerProducedDataHasRunIdleHeader) && (data_size > 0) && (!is_heartbeat))
            {
                // we only have an run idle header if it is not an heartbeat connection
                data_size -= 4; // remove the 4 bytes needed for run/idle header
                diff_size += 4;
            }

            if (((CipByteArray*)attribute->getData())->length != data_size)
            {
                /*wrong connection size*/
                instance->correct_target_to_originator_size = (CipUint) (((CipByteArray*)attribute->getData())->length + diff_size);
                *extended_error = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize;
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        if (nullptr != g_config_data_buffer)
        {
            // config data has been sent with this forward open request
            *extended_error = HandleConfigData((CIP_Assembly*)assembly_class);
            if (0 != *extended_error)
            {
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        eip_status = OpenCommunicationChannels();
        if (kCipStatusOk != eip_status.status)
        {
            *extended_error = 0; //TODO find out the correct extended error code
            return eip_status;
        }
    }

    CIP_ConnectionManager::AddNewActiveConnection(connection_instance);
    //todo:CheckIoConnectionEvent(connection_manager->connection_path.connection_point[0], connection_manager->connection_path.connection_point[1], kIoConnectionEventOpened);
    return eip_status;
}

/*   @brief Open a Point2Point connection dependent on pa_direction.
 *   @param this Pointer to registered Object in ConnectionManager.
 *   @param cpf_data Index of the connection object
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus CIP_ConnectionManager::OpenConsumingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    //static EIP_UINT16 nUDPPort = 2222; //TODO think on improving the udp port assigment for point to point connections
    int j = 0;
    struct sockaddr_in *addr = new struct sockaddr_in();
    int socket;

    if (cpf_data->address_info_item[0].type_id == 0)
    {
        // it is not used yet
        j = 0;
    }
    else if (cpf_data->address_info_item[1].type_id == 0)
    {
        j = 1;
    }

    ///todo: create UDP socket only with CIP_Connectio
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    //addr.in_port = htons(nUDPPort++);
    addr->sin_port = NET_Connection::endian_htons((uint16_t) kOpENerEipIoUdpPort);

    // the address is only needed for bind used if consuming
    netConn->SetSocketHandle (NET_NetworkHandler::CreateUdpSocket(kUdpCommuncationDirectionConsuming, (struct sockaddr*)&addr));
    if (netConn->GetSocketHandle () == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenPointToPointConnection\n");
        return kCipStatusError;
    }

    netConn->originator_address = (struct sockaddr *)addr; // store the address of the originator for packet scanning

    cpf_data->address_info_item[j].length = 16;
    cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget;

    cpf_data->address_info_item[j].sin_port = addr->sin_port;
    //TODO should we add our own address here?
    cpf_data->address_info_item[j].sin_addr = addr->sin_addr.s_addr;
    memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
    cpf_data->address_info_item[j].sin_family = NET_Connection::endian_htons(AF_INET);

    return kCipStatusOk;
}

CipStatus CIP_ConnectionManager::OpenProducingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    int socket;
    in_port_t port = NET_Connection::endian_htons((uint16_t) kOpENerEipIoUdpPort); /* the default port to be used if no port information is part of the forward open request */

    if (CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator == cpf_data->address_info_item[0].type_id)
    {
        port = cpf_data->address_info_item[0].sin_port;
    }
    else
    {
        if (CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator == cpf_data->address_info_item[1].type_id)
        {
            port = cpf_data->address_info_item[1].sin_port;
        }
    }

    ((struct sockaddr_in*)(netConn->remote_address))->sin_family = AF_INET;
    // we don't know the address of the originate will be set in the IApp_CreateUDPSocket
    ((struct sockaddr_in*)(netConn->remote_address))->sin_addr.s_addr = 0;
    ((struct sockaddr_in*)(netConn->remote_address))->sin_port = port;

    socket = NET_NetworkHandler::CreateUdpSocket (kUdpCommuncationDirectionProducing, (struct sockaddr*)&netConn->remote_address); /* the address is only needed for bind used if consuming */

    if (socket == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenPointToPointConnection\n");
        // *pa_pnExtendedError = 0x0315; //miscellaneous
        return (CipStatus) kCipErrorConnectionFailure;
    }
    netConn->SetSocketHandle (socket);

    return kCipStatusOk;
}

CipStatus CIP_ConnectionManager::OpenProducingMulticastConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    CIP_Connection* existing_connection_object = (CIP_Connection*)CIP_AppConnType::GetExistingProducerMulticastConnection(connection_manager->connection_path.connection_point[1]);
    int j;

    if (nullptr == existing_connection_object)
    {
        // we are the first connection producing for the given Input Assembly
        return OpenMulticastConnection(kUdpCommuncationDirectionProducing, cpf_data);
    }
    else
    {
        // we need to inform our originator on the correct connection id
        CIP_produced_connection_id = existing_connection_object->CIP_produced_connection_id;
    }

    // we have a connection reuse the data and the socket

    // allocate an unused sockaddr struct to use
    j = 0;

    if (CIP_CommonPacket::common_packet_data.address_info_item[0].type_id == 0)
    {
        // it is not used yet
        j = 0;
    }
    else if (CIP_CommonPacket::common_packet_data.address_info_item[1].type_id == 0)
    {
        j = 1;
    }

    if (CIP_Connection::kConnectionTypeIoExclusiveOwner == Instance_type)
    {
        //exclusive owners take the socket and further manage the connection
        //especially in the case of time outs.

        netConn->SetSocketHandle (existing_connection_object->netConn->GetSocketHandle ());
        existing_connection_object->netConn->SetSocketHandle (kEipInvalidSocket);
    }
    else
    {
        // this connection will not produce the data
        netConn->SetSocketHandle (kEipInvalidSocket);
    }

    cpf_data->address_info_item[j].length = 16;
    cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator;
    ((struct sockaddr_in*)(netConn->remote_address))->sin_family = AF_INET;
    ((struct sockaddr_in*)(netConn->remote_address))->sin_port = cpf_data->address_info_item[j].sin_port = NET_Connection::endian_htons(
            (uint16_t) kOpENerEipIoUdpPort);
    ((struct sockaddr_in*)(netConn->remote_address))->sin_addr.s_addr = cpf_data->address_info_item[j].sin_addr = CIP_TCPIP_Interface::g_multicast_configuration.starting_multicast_address;
    memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
    cpf_data->address_info_item[j].sin_family = NET_Connection::endian_htons(AF_INET);

    return kCipStatusOk;
}

/**  @brief Open a Multicast connection dependent on @var direction.
 *
 *   @param direction Flag to indicate if consuming or producing.
 *   @param this  pointer to registered Object in ConnectionManager.
 *   @param cpf_data     received CPF Data Item.
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus CIP_ConnectionManager::OpenMulticastConnection(UdpCommuncationDirection direction, CIP_CommonPacket::PacketFormat* cpf_data)
{
    int j = 0;
    int socket;

    if (0 != CIP_CommonPacket::common_packet_data.address_info_item[0].type_id) {
        if ((kUdpCommuncationDirectionConsuming == direction) && (CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget == cpf_data->address_info_item[0].type_id))
        {
            // for consuming connection points the originator can choose the multicast address to use
            // we have a given address type so use it
        }
        else
        {
            j = 1;
            /* if the type is not zero (not used) or if a given type it has to be the correct one */
            if ((0 != CIP_CommonPacket::common_packet_data.address_info_item[1].type_id) && (!((kUdpCommuncationDirectionConsuming == direction)
                                                                                               && (CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget == cpf_data->address_info_item[0].type_id))))
            {
                OPENER_TRACE_ERR("no suitable addr info item available\n");
                return kCipStatusError;
            }
        }
    }

    if (0 == cpf_data->address_info_item[j].type_id)
    {
        // we are using an unused item initialize it with the default multicast address
        cpf_data->address_info_item[j].sin_family = NET_Connection::endian_htons(AF_INET);
        cpf_data->address_info_item[j].sin_port = NET_Connection::endian_htons((uint16_t) kOpENerEipIoUdpPort);
        cpf_data->address_info_item[j].sin_addr = CIP_TCPIP_Interface::g_multicast_configuration.starting_multicast_address;
        memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
        cpf_data->address_info_item[j].length = 16;
    }

    if (NET_Connection::endian_htons(AF_INET) != cpf_data->address_info_item[j].sin_family)
    {
        OPENER_TRACE_ERR("Sockaddr Info Item with wrong sin family value recieved\n");
        return kCipStatusError;
    }

    /* allocate an unused sockaddr struct to use */
    struct sockaddr_in *socket_address = new struct sockaddr_in();
    socket_address->sin_family = NET_Connection::endian_ntohs((uint16_t) cpf_data->address_info_item[j].sin_family);
    socket_address->sin_addr.s_addr = cpf_data->address_info_item[j].sin_addr;
    socket_address->sin_port = cpf_data->address_info_item[j].sin_port;

    // the address is only needed for bind used if consuming
    netConn->SetSocketHandle ( NET_NetworkHandler::CreateUdpSocket (direction, (struct sockaddr*)socket_address));

    if (netConn->GetSocketHandle () == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenMulticastConnection\n");
        return kCipStatusError;
    }

    if (direction == kUdpCommuncationDirectionConsuming)
    {
        cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget;
        netConn->originator_address = (struct sockaddr*)socket_address;
    }
    else
    {
        cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator;
        netConn->remote_address = (struct sockaddr*)socket_address;
    }

    return kCipStatusOk;
}

CipUint CIP_ConnectionManager::HandleConfigData(CIP_Assembly* assembly_class)
{
    CipUint connection_manager_status = 0;
    const CIP_Object* config_instance = (const CIP_Object<CIP_ConnectionManager>*)CIP_Assembly::GetClass();

    if (0 != g_config_data_length)
    {
        if (CIP_AppConnType::ConnectionWithSameConfigPointExists(connection_manager->connection_path.connection_point[2]))
        {
            // there is a connected connection with the same config point
            // we have to have the same data as already present in the config point
            CipByteArray* p = (CipByteArray*)GetCipAttribute(3)->getData ();
            if (p->length != g_config_data_length)
            {
                connection_manager_status = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorOwnershipConflict;
            }
            else
            {
                /*FIXME check if this is correct */
                if (memcmp(p->data, g_config_data_buffer, g_config_data_length))
                {
                    connection_manager_status = CIP_ConnectionManager::kConnectionManagerStatusCodeErrorOwnershipConflict;
                }
            }
        }
        else
        {
            // put the data on the configuration assembly object with the current
            // design this can be done rather efficiently
            /*todo:
            if (kCipStatusOk != NotifyAssemblyConnectedDataReceived(config_instance, g_config_data_buffer, g_config_data_length))
            {
                OPENER_TRACE_WARN("Configuration data was invalid\n");
                connection_manager_status = CIP_ConnectionManager::kConnectionManagerStatusCodeInvalidConfigurationApplicationPath;
            }
             */
        }
    }
    return connection_manager_status;
}

void CIP_ConnectionManager::CloseIoConnection()
{

    //todo:CheckIoConnectionEvent(connection_manager->connection_path.connection_point[0], connection_manager->connection_path.connection_point[1], kIoConnectionEventClosed);

    if ((CIP_Connection::kConnectionTypeIoExclusiveOwner == Instance_type)
        || (CIP_Connection::kConnectionTypeIoInputOnly == Instance_type))
    {
        if ((CIP_ConnectionManager::kRoutingTypeMulticastConnection == (t_to_o_network_connection_parameter & CIP_ConnectionManager::kRoutingTypeMulticastConnection)) && (kEipInvalidSocket != netConn->sock))
        {
            CIP_Connection* next_non_control_master_connection = (CIP_Connection*)CIP_AppConnType::GetNextNonControlMasterConnection(connection_manager->connection_path.connection_point[1]);
            if (nullptr != next_non_control_master_connection)
            {
                next_non_control_master_connection->netConn->SetSocketHandle (netConn->GetSocketHandle ());

                memcpy(&(next_non_control_master_connection->netConn->remote_address),
                       &(netConn->remote_address),
                       sizeof(next_non_control_master_connection->netConn->remote_address));

                next_non_control_master_connection->eip_level_sequence_count_producing = eip_level_sequence_count_producing;

                next_non_control_master_connection->sequence_count_producing = sequence_count_producing;
                netConn->SetSocketHandle (kEipInvalidSocket);
                next_non_control_master_connection->transmission_trigger_timer = transmission_trigger_timer;
            }
            else
            {
                // this was the last master connection close all listen only connections listening on the port
                CIP_AppConnType::CloseAllConnectionsForInputWithSameType(connection_manager->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
            }
        }
    }

    CloseCommunicationChannelAndRemoveFromActiveConnectionsList();
}

void CIP_ConnectionManager::HandleIoConnectionTimeOut()
{
    CIP_Connection* next_non_control_master_connection;
    //todo:CheckIoConnectionEvent(connection_manager->connection_path.connection_point[0], connection_manager->connection_path.connection_point[1], kIoConnectionEventTimedOut);

    CIP_ConnectionManager* connection_manager = CIP_ConnectionManager::GetConnectionManagerObject (next_non_control_master_connection->id);

    if (CIP_ConnectionManager::kRoutingTypeMulticastConnection == (connection_manager->t_to_o_network_connection_parameter & CIP_ConnectionManager::kRoutingTypeMulticastConnection))
    {
        switch (Instance_type)
        {
            case CIP_Connection::kConnectionTypeIoExclusiveOwner:
                CIP_AppConnType::CloseAllConnectionsForInputWithSameType(connection_manager->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoInputOnly);
                CIP_AppConnType::CloseAllConnectionsForInputWithSameType(connection_manager->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
                break;
            case CIP_Connection::kConnectionTypeIoInputOnly:
                if (kEipInvalidSocket != netConn->GetSocketHandle ())
                {
                    // we are the controlling input only connection find a new controller
                    next_non_control_master_connection = (CIP_Connection*)CIP_AppConnType::GetNextNonControlMasterConnection(connection_manager->connection_path.connection_point[1]);
                    if (nullptr != next_non_control_master_connection)
                    {
                        next_non_control_master_connection->netConn->SetSocketHandle (netConn->GetSocketHandle ());
                        netConn->SetSocketHandle (kEipInvalidSocket);
                        next_non_control_master_connection->transmission_trigger_timer = transmission_trigger_timer;
                    }
                    else
                    {
                        //this was the last master connection close all listen only connections listening on the port
                        CIP_AppConnType::CloseAllConnectionsForInputWithSameType(connection_manager->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
                    }
                }
                break;
            default:
                break;
        }
    }

    //OPENER_ASSERT(nullptr != CloseConnection ());
    CloseConnection();
}

CipStatus CIP_ConnectionManager::SendConnectedData()
{
    CIP_CommonPacket::PacketFormat* cpf_data;
    CipUint reply_length;
    CipUsint* message_data_reply_buffer;

    /* TODO think of adding an own send buffer to each connection object in order to preset up the whole message on connection opening and just change the variable data items e.g., sequence number */

    cpf_data = &CIP_CommonPacket::common_packet_data; /* TODO think on adding a CPF data item to the S_CIP_CIP_Connection in order to remove the code here or even better allocate memory in the connection object for storing the message to send and just change the application data*/

    eip_level_sequence_count_producing++;

    /* assembleCPFData */
    cpf_data->item_count = 2;
    if ((transport_type_class_trigger & 0x0F) != 0)
    {
        // use Sequenced Address Items if not Connection Class 0
        cpf_data->address_item.type_id = CIP_CommonPacket::kCipItemIdSequencedAddressItem;
        cpf_data->address_item.length = 8;
        cpf_data->address_item.data.sequence_number = eip_level_sequence_count_producing;
    }
    else
    {
        cpf_data->address_item.type_id = CIP_CommonPacket::kCipItemIdConnectionAddress;
        cpf_data->address_item.length = 4;
    }
    cpf_data->address_item.data.connection_identifier = CIP_produced_connection_id;

    cpf_data->data_item.type_id = CIP_CommonPacket::kCipItemIdConnectedDataItem;

    CipByteArray* producing_instance_attributes = (CipByteArray*)(((CIP_Object*)producing_instance)->attributes[0]->getData());
    cpf_data->data_item.length = 0;

    //todo: verify data before sending
    // notify the application that data will be sent immediately after the call
    /*
    if (BeforeAssemblyDataSend(producing_instance))
    {
        // the data has changed increase sequence counter
        sequence_count_producing++;
    }
     */

    // set AddressInfo Items to invalid Type
    cpf_data->address_info_item[0].type_id = 0;
    cpf_data->address_info_item[1].type_id = 0;

    reply_length = (CipUint) CIP_CommonPacket::AssembleIOMessage(cpf_data, (CipUsint*)CIP_Common::message_data_reply_buffer[0]);

    message_data_reply_buffer = (CipUsint*)CIP_Common::message_data_reply_buffer[reply_length - 2];
    cpf_data->data_item.length = producing_instance_attributes->length;

    if (kOpENerProducedDataHasRunIdleHeader)
    {
        cpf_data->data_item.length += 4;
    }

    if ((transport_type_class_trigger & 0x0F) == 1)
    {
        cpf_data->data_item.length += 2;
        NET_Endianconv::AddIntToMessage(cpf_data->data_item.length, message_data_reply_buffer);
        NET_Endianconv::AddIntToMessage(sequence_count_producing, message_data_reply_buffer);
    }
    else
    {
        NET_Endianconv::AddIntToMessage(cpf_data->data_item.length, message_data_reply_buffer);
    }

    if (kOpENerProducedDataHasRunIdleHeader)
    {
        NET_Endianconv::AddDintToMessage(g_run_idle_state, &(message_data_reply_buffer));
    }

    memcpy(message_data_reply_buffer, producing_instance_attributes->data, producing_instance_attributes->length);

    reply_length += cpf_data->data_item.length;
    return NET_NetworkHandler::SendUdpData(netConn->remote_address, netConn->sock, (CipUsint*)CIP_Common::message_data_reply_buffer[0], reply_length);
}

CipStatus CIP_ConnectionManager::HandleReceivedIoConnectionData(CipUsint* data, CipUint data_length)
{

    /* check class 1 sequence number*/
    if ((transport_type_class_trigger & 0x0F) == 1)
    {
        CipUint sequence_buffer = NET_Endianconv::GetIntFromMessage(data);
        if (SEQ_LEQ16(sequence_buffer, sequence_count_consuming))
        {
            return kCipStatusOk; // no new data for the assembly
        }
        sequence_count_consuming = sequence_buffer;
        data_length -= 2;
    }

    if (data_length > 0) {
        // we have no heartbeat connection
        if (kOpENerConsumedDataHasRunIdleHeader)
        {
            CipUdint nRunIdleBuf = NET_Endianconv::GetDintFromMessage(data);
            //todo: update idle state
            /*
            if (g_run_idle_state != nRunIdleBuf)
            {
                RunIdleChanged(nRunIdleBuf);
            }
            */
            g_run_idle_state = nRunIdleBuf;
            data_length -= 4;
        }

        /*todo:
        if (NotifyAssemblyConnectedDataReceived(consuming_instance, data, data_length) != 0)
        {
            return kCipStatusError;
        }
         */
    }
    return kCipStatusOk;
}

CipStatus CIP_ConnectionManager::OpenCommunicationChannels()
{

    CipStatus eip_status = kCipStatusOk;
    CIP_ConnectionManager * connection_manager = CIP_ConnectionManager::GetConnectionManagerObject (this->id);

    //get pointer to the CPF data, currently we have just one global instance of the struct. This may change in the future
    CIP_CommonPacket::PacketFormat* cpf_data = &CIP_CommonPacket::common_packet_data;

    int originator_to_target_connection_type = (connection_manager->o_to_t_network_connection_parameter & 0x6000) >> 13;

    int target_to_originator_connection_type = (connection_manager->t_to_o_network_connection_parameter & 0x6000) >> 13;

    // open a connection "point to point" or "multicast" based on the ConnectionParameter
    if (originator_to_target_connection_type == 1) //TODO: Fix magic number; Multicast consuming
    {
        if (kCipStatusError == OpenMulticastConnection(kUdpCommuncationDirectionConsuming, cpf_data).status)
        {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    } else if (originator_to_target_connection_type == 2) // TODO: Fix magic number; Point to Point consuming
    {
        if (kCipStatusError == OpenConsumingPointToPointConnection(cpf_data).status)
        {
            OPENER_TRACE_ERR("error in PointToPoint consuming connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }

    if (target_to_originator_connection_type == 1) // TODO: Fix magic number; Multicast producing
    {
        if (kCipStatusError == OpenProducingMulticastConnection(cpf_data).status)
        {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }
    else if (target_to_originator_connection_type == 2) // TODO: Fix magic number; Point to Point producing
    {

        if (kCipStatusOk == OpenProducingPointToPointConnection(cpf_data).status)
        {
            OPENER_TRACE_ERR("error in PointToPoint producing connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }

    return eip_status;
}

void CIP_ConnectionManager::CloseCommunicationChannelAndRemoveFromActiveConnectionsList()
{
    netConn->CloseSocket ();
    RemoveFromActiveConnections();
}

/*END OF TEMPORARY IO CONN STUFF*/

/** @brief gets the padded logical path TODO: enhance documentation
 * @param logical_path_segment TheLogical Path Segment
 *
 */
unsigned int CIP_ConnectionManager::GetPaddedLogicalPath (unsigned char *logical_path_segment)
{
    unsigned int padded_logical_path = *(logical_path_segment)++;

    if ((padded_logical_path & 3) == 0)
    {
        padded_logical_path = *(logical_path_segment)++;
    }
    else if ((padded_logical_path & 3) == 1)
    {
        (*logical_path_segment)++; // skip pad
        padded_logical_path = *(logical_path_segment)++;
        padded_logical_path |= *(logical_path_segment)++ << 8;
    }
    else
    {
        OPENER_TRACE_ERR("illegal logical path segment\n");
    }
    return padded_logical_path;
}

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

CipStatus CIP_ConnectionManager::HandleReceivedConnectedData (CIP_ConnectionManager * connection_manager_instance,
                                                              CipUsint *data,
                                                              int data_length,
                                                              struct sockaddr_in *from_address
                                                            )
{

    CIP_Connection * connection_object
            = (CIP_Connection*)CIP_Connection::GetInstance(connection_manager_instance->connection_serial_number);

    if (kCipStatusError == (CIP_CommonPacket::CreateCommonPacketFormatStructure
            (data, data_length, &CIP_CommonPacket::common_packet_data)).status)
    {
        return kCipStatusError;
    }

    // check if connected address item or sequenced address item  received, otherwise it is no connected message and should not be here
    if ((CIP_CommonPacket::common_packet_data.address_item.type_id == CIP_CommonPacket::kCipItemIdConnectionAddress) || (CIP_CommonPacket::common_packet_data.address_item.type_id == CIP_CommonPacket::kCipItemIdSequencedAddressItem))
    {
        // found connected address item or found sequenced address item -> for now the sequence number will be ignored
        if (CIP_CommonPacket::common_packet_data.data_item.type_id == CIP_CommonPacket::kCipItemIdConnectedDataItem)
        {
            // connected data item received
            //CIP_ConnectionManager *connection_object = GetConnectedObject (CIP_CommonPacket::common_packet_data.address_item.data.connection_identifier);
            //if (connection_object == nullptr)
            //    return kCipStatusError;

            // only handle the data if it is coming from the originator
            if (((struct sockaddr_in*)(connection_object->netConn->originator_address))->sin_addr.s_addr == from_address->sin_addr.s_addr)
            {
                if (SEQ_GT32(CIP_CommonPacket::common_packet_data.address_item.data.sequence_number, connection_manager_instance->eip_level_sequence_count_consuming))
                {
                    // reset the watchdog timer
                    connection_manager_instance->inactivity_watchdog_timer
                            = (connection_manager_instance->o_to_t_requested_packet_interval / 1000)
                            << (2 + connection_manager_instance->connection_timeout_multiplier);

                    // only inform assembly object if the sequence counter is greater or equal
                    connection_manager_instance->eip_level_sequence_count_consuming = CIP_CommonPacket::common_packet_data.address_item.data.sequence_number;

                    //TODO: fix handles per IO Type
                    //return HandleReceivedIoConnectionData (CIP_CommonPacket::common_packet_data.data_item.data, CIP_CommonPacket::common_packet_data.data_item.length);
                }
            }
            else
            {
                OPENER_TRACE_WARN("Connected Message Data Received with wrong address information\n");
            }
        }
    }

    return kCipStatusOk;
}

/*   @brief Check if resources for new connection available, generate ForwardOpen Reply message.
 *      message_router_request		pointer to Message Router Request.
 *      message_router_response		pointer to Message Router Response.
 * 		@return >0 .. success, 0 .. no reply to send back
 *      	-1 .. error
 */
CipStatus CIP_ConnectionManager::ForwardOpen (CipMessageRouterRequest_t *message_router_request,
                                              CipMessageRouterResponse_t *message_router_response)
{
    CipUint connection_status = kConnectionManagerStatusCodeSuccess;
    ConnectionManagementHandling *connection_management_entry;
    
    //first check if we have already a connection with the given params
    //todo: fix priority_timetick = &message_router_request->request_data[0]++;
    //todo: fix timeout_ticks = &message_router_request->request_data[0]++;
    // O_to_T netConn ID
    connection_object->CIP_consumed_connection_id = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);
    // T_to_O netConn ID
    connection_object->CIP_produced_connection_id = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);
    connection_serial_number = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);
    originator_vendor_id = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);
    originator_serial_number = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);

    if ((nullptr != CheckForExistingConnection (g_dummy_connection_object)))
    {
        // TODO this test is  incorrect, see CIP spec 3-5.5.2 re: duplicate forward open
        // it should probably be testing the connection type fields
        // TODO think on how a reconfiguration request could be handled correctly
        if ((0 == connection_object->CIP_consumed_connection_id) && (0 == connection_object->CIP_produced_connection_id))
        {
            //TODO implement reconfiguration of connection

            OPENER_TRACE_ERR("this looks like a duplicate forward open -- I can't handle this yet, sending a CIP_CON_MGR_ERROR_CONNECTION_IN_USE response\n");
        }
        return AssembleForwardOpenResponse (connection_object,
                                            message_router_response,
                                            kCipErrorConnectionFailure,
                                            kConnectionManagerStatusCodeErrorConnectionInUse
                                            );
    }
    // keep it to none existent till the setup is done this eases error handling and
    // the state changes within the forward open request can not be detected from
    // the application or from outside (reason we are single threaded)
    connection_object->State = CIP_Connection::kConnectionStateNonExistent;
    sequence_count_producing = 0; // set the sequence count to zero

    connection_timeout_multiplier = message_router_request->request_data[0]++;
    //message_router_request->request_data += 3; // reserved
    // the requested packet interval parameter needs to be a multiple of TIMERTICK from the header file
    OPENER_TRACE_INFO("ForwardOpen: ConConnID %" PRIu32 ", ProdConnID %" PRIu32 ", ConnSerNo %u\n", consumed_connection_id, produced_connection_id, connection_serial_number);

    o_to_t_requested_packet_interval = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);

    o_to_t_network_connection_parameter = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);
    t_to_o_requested_packet_interval = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);

    CipUdint temp = t_to_o_requested_packet_interval % (kOpENerTimerTickInMilliSeconds * 1000);
    if (temp > 0)
    {
        t_to_o_requested_packet_interval = (CipUdint) (t_to_o_requested_packet_interval / (kOpENerTimerTickInMilliSeconds * 1000))
                                           * (kOpENerTimerTickInMilliSeconds * 1000) + (kOpENerTimerTickInMilliSeconds * 1000);
    }

    t_to_o_network_connection_parameter = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);

    //check if Network connection parameters are ok
    if (CIP_CONN_TYPE_MASK == (o_to_t_network_connection_parameter & CIP_CONN_TYPE_MASK))
    {
        return AssembleForwardOpenResponse ( connection_object,
                                             message_router_response,
                                             kCipErrorConnectionFailure,
                                             kConnectionManagerStatusCodeErrorInvalidOToTConnectionType
                                            );
    }

    if (CIP_CONN_TYPE_MASK == (t_to_o_network_connection_parameter & CIP_CONN_TYPE_MASK))
    {
        return AssembleForwardOpenResponse ( connection_object,
                                             message_router_response,
                                             kCipErrorConnectionFailure,
                                             kConnectionManagerStatusCodeErrorInvalidTToOConnectionType
                                            );
    }

    //todo: fix transport_type_class_trigger = &message_router_request->request_data[0]++;
    //check if the trigger type value is ok
    if (0x40 & producing_instance->TransportClass_trigger.bitfield_u.production_trigger)
    {
        return AssembleForwardOpenResponse ( connection_object,
                                             message_router_response,
                                             kCipErrorConnectionFailure,
                                             kConnectionManagerStatusCodeErrorTransportTriggerNotSupported
                                            );
    }

    temp = ParseConnectionPath ( message_router_request, &connection_status);
    if (kCipStatusOk != temp)
    {
        return AssembleForwardOpenResponse ( connection_object,
                                             message_router_response,
                                             (CipUsint)temp,
                                             connection_status
                                            );
    }

    //parsing is now finished all data is available and check now establish the connection
    connection_management_entry = GetConnMgmEntry (connection_path.class_id);
    if (nullptr != connection_management_entry)
    {
        CipUint * extended_error;
        //todo:temp = connection_management_entry->open_connection_function ( &connection_status, extended_error);
    }
    else
    {
        temp = kCipStatusError;
        connection_status = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
    }

    if (kCipStatusOk != temp)
    {
        OPENER_TRACE_INFO("connection manager: connect failed\n");
        // in case of error the dummy objects holds all necessary information
        return AssembleForwardOpenResponse (connection_object, message_router_response, (CipUsint)temp, connection_status);
    }
    else
    {
        OPENER_TRACE_INFO("connection manager: connect succeeded\n");
        // in case of success the g_pstActiveConnectionList points to the new connection
        return AssembleForwardOpenResponse (connection_object, message_router_response, kCipErrorSuccess, 0);
    }
}

void CIP_ConnectionManager::GeneralConnectionConfiguration (CIP_ConnectionManager * connection_manager_instance)
{
    CIP_Connection * connection_instance = (CIP_Connection*)CIP_Connection::GetInstance(connection_manager_instance->connection_serial_number);
    if (kRoutingTypePointToPointConnection == (connection_manager_instance->o_to_t_network_connection_parameter & kRoutingTypePointToPointConnection))
    {
        // if we have a point to point connection for the O to T direction the target shall choose the connection ID.
        connection_instance->CIP_consumed_connection_id = GetConnectionId ();
    }

    if (kRoutingTypeMulticastConnection == (connection_manager_instance->t_to_o_network_connection_parameter & kRoutingTypeMulticastConnection))
    {
        // if we have a multi-cast connection for the T to O direction the target shall choose the connection ID.
        connection_instance->CIP_produced_connection_id = GetConnectionId ();
    }

    connection_manager_instance->eip_level_sequence_count_producing = 0;
    connection_manager_instance->sequence_count_producing = 0;
    connection_manager_instance->eip_level_sequence_count_consuming = 0;
    connection_manager_instance->sequence_count_consuming = 0;

    connection_instance->Watchdog_timeout_action = CIP_Connection::kWatchdogTimeoutActionAutoDelete; /* the default for all connections on EIP*/

    connection_instance->Expected_packet_rate = 0; // default value

    // Client Type Connection requested
    if (connection_manager_instance->consuming_instance->TransportClass_trigger.bitfield_u.direction
        == CIP_Connection::kConnectionTriggerDirectionClient)
    {
        connection_instance->Expected_packet_rate = (CipUint) ((connection_manager_instance->t_to_o_requested_packet_interval) / 1000);
        // As soon as we are ready we should produce the connection. With the 0 here we will produce with the next timer tick which should be sufficient.
        connection_manager_instance->transmission_trigger_timer = 0;
    }
    else
    {
        // Server Type Connection requested
        connection_instance->Expected_packet_rate = (CipUint) ((connection_manager_instance->o_to_t_requested_packet_interval) / 1000);
    }

    connection_manager_instance->production_inhibit_timer = connection_manager_instance->production_inhibit_time = 0;

    //setup the preconsuption timer: max(ConnectionTimeoutMultiplier * EpectetedPacketRate, 10s)
    connection_manager_instance->inactivity_watchdog_timer
            = ((((connection_manager_instance->o_to_t_requested_packet_interval) / 1000)
            << (2 + connection_manager_instance->connection_timeout_multiplier)) > 10000)
              ? (((connection_manager_instance->o_to_t_requested_packet_interval) / 1000)
                    << (2 + connection_manager_instance->connection_timeout_multiplier)) : 10000;

    connection_instance->Consumed_connection_size = (CipUint)(connection_manager_instance->o_to_t_network_connection_parameter & 0x01FF);

    connection_instance->Produced_connection_size = (CipUint)(connection_manager_instance->t_to_o_network_connection_parameter & 0x01FF);
}

void CIP_ConnectionManager::GeneralConnectionConfiguration(CIP_Connection * connection_instance)
{
    //Search connection_manager_instance managing the connection instance
    CIP_ConnectionManager * conn_mgr = GetConnectionManagerObject (connection_instance->id);

    return GeneralConnectionConfiguration (conn_mgr);
}

CipStatus CIP_ConnectionManager::ForwardClose (CipMessageRouterRequest_t *message_router_request, CipMessageRouterResponse_t *message_router_response)
{

    // check connection_serial_number && originator_vendor_id && originator_serial_number if connection is established
    ConnectionManagerStatusCode_e connection_status = kConnectionManagerStatusCodeErrorConnectionNotFoundAtTargetApplication;
    CIP_ConnectionManager *connection_manager_instance;
    CIP_Connection * connection_instance;

    // set AddressInfo Items to invalid TypeID to prevent assembleLinearMsg to read them
    CIP_CommonPacket::common_packet_data.address_info_item[0].type_id = 0;
    CIP_CommonPacket::common_packet_data.address_info_item[1].type_id = 0;

    //message_router_request->request_data += 2; // ignore Priority/Time_tick and Time-out_ticks

    CipUint connection_serial_number = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);
    CipUint originator_vendor_id = NET_Endianconv::GetIntFromMessage (&message_router_request->request_data[0]);
    CipUdint originator_serial_number = NET_Endianconv::GetDintFromMessage (&message_router_request->request_data[0]);

    OPENER_TRACE_INFO("ForwardClose: ConnSerNo %d\n", connection_serial_number);

    for (unsigned int i = 0; i < active_connections_set->size(); i++)
    {
        connection_manager_instance = (CIP_ConnectionManager*)active_connections_set->at(i);
        connection_instance = (CIP_Connection*)CIP_Connection::GetInstance (connection_manager_instance->connection_serial_number);
        /* this check should not be necessary as only established connections should be in the active connection list */
        if ((connection_instance->State == CIP_Connection::kConnectionStateEstablished)
            || (connection_instance->State == CIP_Connection::kConnectionStateTimedOut))
        {
            if ((connection_manager_instance->connection_serial_number == connection_serial_number) &&
                (connection_manager_instance->originator_vendor_id == originator_vendor_id) &&
                (connection_manager_instance->originator_serial_number == originator_serial_number))
            {
                /* found the corresponding connection object -> close it */
                //OPENER_ASSERT(nullptr != connection_object->connection_close_function);
                CloseConnection (connection_manager_instance);
                connection_status = kConnectionManagerStatusCodeSuccess;
                break;
            }
        }
    }

    return AssembleForwardCloseResponse (connection_serial_number, originator_vendor_id, originator_serial_number, message_router_request, message_router_response, connection_status);
}

ConnectionManagementHandling* CIP_ConnectionManager::GetConnMgmEntry(CipUdint class_id)
{
    int i;
    ConnectionManagementHandling *pstRetVal;

    pstRetVal = nullptr;

    for (i = 0; i < g_kNumberOfConnectableObjects; ++i)
    {
        if (class_id == g_astConnMgmList->at(i).class_id)
        {
            pstRetVal = &(g_astConnMgmList->at(i));
            break;
        }
    }
    return pstRetVal;
}


/* TODO: Not implemented */
CipStatus CIP_ConnectionManager::GetConnectionOwner (CipMessageRouterRequest_t *message_router_request, CipMessageRouterResponse_t *message_router_response)
{
    /* suppress compiler warnings */
    (void) message_router_request;
    (void) message_router_response;

    return kCipStatusOk;
}

CipStatus CIP_ConnectionManager::ManageConnections (MilliSeconds elapsed_time)
{
    CipStatus eip_status;
    CIP_ConnectionManager *connection_manager_instance;
    CIP_Connection * connection_instance;

    /*Inform application that it can execute */
    //todo:HandleApplication ();
    NET_EthIP_Encap::ManageEncapsulationMessages (elapsed_time);

    for (unsigned int i = 0; i < active_connections_set->size (); i++)
    {
        connection_manager_instance = (CIP_ConnectionManager*)object_Set[i];
        connection_instance = (CIP_Connection*)CIP_Connection::GetInstance (connection_manager_instance->connection_serial_number);

        if (connection_instance->State == CIP_Connection::kConnectionStateEstablished)
        {
            // we have a consuming connection check inactivity watchdog timer or all sever connections have to maintain an inactivity watchdog timer
            if ( (0 != connection_manager_instance->consuming_instance)
                ||  (connection_manager_instance->consuming_instance->TransportClass_trigger.bitfield_u.direction
                     == CIP_Connection::kConnectionTriggerDirectionClient) )
            {
                connection_manager_instance->inactivity_watchdog_timer -= elapsed_time;
                if (connection_manager_instance->inactivity_watchdog_timer <= 0)
                {
                    // we have a timed out connection perform watchdog time out action
                    OPENER_TRACE_INFO(">>>>>>>>>>Connection timed out\n");
                    //OPENER_ASSERT(nullptr != connection_instance->connection_timeout_function);
                    RemoveFromActiveConnections (connection_manager_instance);
                }
            }
            // only if the connection has not timed out check if data is to be send
            if (CIP_Connection::kConnectionStateEstablished == connection_instance->State)
            {
                // client connection
                if ((connection_instance->Expected_packet_rate != 0)
                    && (kEipInvalidSocket != connection_instance->netConn->sock)) // only produce for the master connection
                {
                    if (connection_manager_instance->producing_instance->TransportClass_trigger.bitfield_u.production_trigger
                        != CIP_Connection::kConnectionTriggerProductionTriggerCyclic)
                    {
                        // non cyclic connections have to decrement production inhibit timer
                        if (0 <= connection_manager_instance->production_inhibit_timer)
                        {
                            connection_manager_instance->production_inhibit_timer -= elapsed_time;
                        }
                    }
                    connection_manager_instance->transmission_trigger_timer -= elapsed_time;
                    if (connection_manager_instance->transmission_trigger_timer <= 0)
                    {
                        // need to send package
                        //OPENER_ASSERT(nullptr != connection_instance->connection_send_data_function);
                        //todo: eip_status = SendConnectedData (connection_instance); //only for IO connections
                        if (eip_status.status == kCipStatusError)
                        {
                            OPENER_TRACE_ERR("sending of UDP data in manage Connection failed\n");
                        }
                        // reload the timer value
                        connection_manager_instance->transmission_trigger_timer = connection_instance->Expected_packet_rate;
                        if (connection_manager_instance->producing_instance->TransportClass_trigger.bitfield_u.production_trigger
                                != CIP_Connection::kConnectionTriggerProductionTriggerCyclic)
                        {
                            // non cyclic connections have to reload the production inhibit timer
                            connection_manager_instance->production_inhibit_timer = connection_manager_instance->production_inhibit_time;
                        }
                    }
                }
            }
        }
    }
    return kCipStatusOk;
}

/* TODO: Update Documentation  INT8 assembleFWDOpenResponse(S_CIP_CIP_Connection *pa_pstConnObj, S_CIP_MR_Response * pa_MRResponse, EIP_UINT8 pa_nGeneralStatus, EIP_UINT16 pa_nExtendedStatus,
 void * deleteMeSomeday, EIP_UINT8 * pa_msg)
 *   create FWDOpen response dependent on status.
 *      pa_pstConnObj pointer to connection Object
 *      pa_MRResponse	pointer to message router response
 *      pa_nGeneralStatus the general status of the response
 *      pa_nExtendedStatus extended status in the case of an error otherwise 0
 *      pa_CPF_data	pointer to CPF Data Item
 *      pa_msg		pointer to memory where reply has to be stored
 *  return status
 * 			0 .. no reply need to be sent back
 * 			1 .. need to send reply
 * 		  -1 .. error
 */
CipStatus CIP_ConnectionManager::AssembleForwardOpenResponse (CIP_Connection * connection_object,
                                                              CipMessageRouterResponse_t *message_router_response,
                                                              CipUsint general_status,
                                                              CipUint extended_status
                                                            )
{
    /* write reply information in CPF struct dependent of pa_status */
    CIP_CommonPacket::PacketFormat *cip_common_packet_format_data = &CIP_CommonPacket::common_packet_data;
    CipByte *message = (CipByte*)&message_router_response->response_data[0];
    cip_common_packet_format_data->item_count = 2;
    cip_common_packet_format_data->data_item.type_id = CIP_CommonPacket::kCipItemIdUnconnectedDataItem;

    AddNullAddressItem (cip_common_packet_format_data);

    message_router_response->reply_service = (0x80 | kForwardOpen);
    message_router_response->general_status = general_status;

    if (kCipErrorSuccess == general_status)
    {
        OPENER_TRACE_INFO("assembleFWDOpenResponse: sending success response\n");
        message_router_response->response_data->reserve (26); /* if there is no application specific data */
        message_router_response->size_additional_status = 0;

        if (cip_common_packet_format_data->address_info_item[0].type_id != 0)
        {
            cip_common_packet_format_data->item_count = 3;
            if (cip_common_packet_format_data->address_info_item[1].type_id != 0)
            {
                cip_common_packet_format_data->item_count = 4; /* there are two sockaddrinfo items to add */
            }
        }

        NET_Endianconv::AddDintToMessage (connection_object->CIP_consumed_connection_id, message);
        NET_Endianconv::AddDintToMessage (connection_object->CIP_produced_connection_id, message);
    }
    else
    {
        /* we have an connection creation error */
        OPENER_TRACE_INFO("assembleFWDOpenResponse: sending error response\n");
        connection_object->State = CIP_Connection::kConnectionStateNonExistent;
        message_router_response->response_data->reserve (10);

        switch (general_status)
        {
            case kCipErrorNotEnoughData:
            case kCipErrorTooMuchData:
            {
                message_router_response->size_additional_status = 0;
                break;
            }

            default:
            {
                switch (extended_status)
                {
                    case kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize:
                    {
                        message_router_response->size_additional_status = 2;
                        message_router_response->additional_status[0] = extended_status;
                        message_router_response->additional_status[1] = correct_originator_to_target_size;
                        break;
                    }

                    case kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize:
                    {
                        message_router_response->size_additional_status = 2;
                        message_router_response->additional_status[0] = extended_status;
                        message_router_response->additional_status[1] = correct_target_to_originator_size;
                        break;
                    }

                    default:
                    {
                        message_router_response->size_additional_status = 1;
                        message_router_response->additional_status[0] = extended_status;
                        break;
                    }
                }
                break;
            }
        }
    }

    NET_Endianconv::AddIntToMessage (connection_serial_number, message);
    NET_Endianconv::AddIntToMessage (originator_vendor_id, message);
    NET_Endianconv::AddDintToMessage (originator_serial_number, message);

    if (kCipErrorSuccess == general_status)
    {
        // set the actual packet rate to requested packet rate
        NET_Endianconv::AddDintToMessage (o_to_t_requested_packet_interval, message);
        NET_Endianconv::AddDintToMessage (t_to_o_requested_packet_interval, message);
    }

    *message = 0; // remaining path size - for routing devices relevant
    message++;
    *message = 0; // reserved
    message++;

    return kCipStatusOkSend; // send reply'
}

/**
 * Adds a Null Address Item to the common data packet format data
 * @param common_data_packet_format_data The CPF data packet where the Null Address Item shall be added
 */
void CIP_ConnectionManager::AddNullAddressItem (CIP_CommonPacket::PacketFormat *common_data_packet_format_data)
{
    // Precondition: Null Address Item only valid in unconnected messages
    assert(common_data_packet_format_data->data_item.type_id == CIP_CommonPacket::kCipItemIdUnconnectedDataItem);

    common_data_packet_format_data->address_item.type_id = CIP_CommonPacket::kCipItemIdNullAddress;
    common_data_packet_format_data->address_item.length = 0;
}

/*   INT8 assembleFWDCloseResponse(UINT16 pa_ConnectionSerialNr, UINT16 pa_OriginatorVendorID, UINT32 pa_OriginatorSerialNr, S_CIP_MR_Request *pa_MRRequest, S_CIP_MR_Response *pa_MRResponse, S_CIP_CPF_Data *pa_CPF_data, INT8 pa_status, INT8 *pa_msg)
 *   create FWDClose response dependent on status.
 *      pa_ConnectionSerialNr	requested ConnectionSerialNr
 *      pa_OriginatorVendorID	requested OriginatorVendorID
 *      pa_OriginatorSerialNr	requested OriginalSerialNr
 *      pa_MRRequest		pointer to message router request
 *      pa_MRResponse		pointer to message router response
 *      pa_CPF_data		pointer to CPF Data Item
 *      pa_status		status of FWDClose
 *      pa_msg			pointer to memory where reply has to be stored
 *  return status
 * 			0 .. no reply need to ne sent back
 * 			1 .. need to send reply
 * 		       -1 .. error
 */
CipStatus CIP_ConnectionManager::AssembleForwardCloseResponse (CipUint connection_serial_number, CipUint originatior_vendor_id, CipUdint originator_serial_number, CipMessageRouterRequest_t *message_router_request, CipMessageRouterResponse_t *message_router_response, CipUint extended_error_code)
{
    // write reply information in CPF struct dependent of pa_status
    CIP_CommonPacket::PacketFormat *common_data_packet_format_data = &CIP_CommonPacket::common_packet_data;
    CipByte *message = (CipByte*)&message_router_response->response_data[0];
    common_data_packet_format_data->item_count = 2;
    common_data_packet_format_data->data_item.type_id = CIP_CommonPacket::kCipItemIdUnconnectedDataItem;

    AddNullAddressItem (common_data_packet_format_data);

    message_router_response->response_data->reserve (10); // if there is no application specific data
    
    NET_Endianconv::AddIntToMessage (connection_serial_number, message);
    NET_Endianconv::AddIntToMessage (originatior_vendor_id, message);
    NET_Endianconv::AddDintToMessage (originator_serial_number, message);

    message_router_response->reply_service = (CipUsint)(0x80 | message_router_request->service);

    if (kConnectionManagerStatusCodeSuccess == extended_error_code)
    {
        *message = 0; // no application data
        message_router_response->general_status = kCipErrorSuccess;
        message_router_response->size_additional_status = 0;
    }
    else
    {
        *message = message_router_request->request_data[0]; // remaining path size
        message_router_response->general_status = kCipErrorConnectionFailure;
        message_router_response->additional_status[0] = extended_error_code;
        message_router_response->size_additional_status = 1;
    }

    message++;
    *message = 0; // reserved
    message++;

    return kCipStatusOkSend;
}

CIP_Connection *CIP_ConnectionManager::GetConnectedObject (CipUdint connection_id)
{
    CIP_Connection *active_connection_object_list_item;;

    for (unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size (); i++)
    {
        active_connection_object_list_item = (CIP_Connection*)active_connections_set->at(i);//todo: fix

        if (active_connection_object_list_item->State == CIP_Connection::kConnectionStateEstablished)
        {
            if (active_connection_object_list_item->CIP_consumed_connection_id == connection_id)
                return active_connection_object_list_item;
        }
    }
    return nullptr;
}

CIP_ConnectionManager *CIP_ConnectionManager::GetConnectionManagerObject (CipUdint connection_id)
{
    CIP_ConnectionManager *active_connection_manager;

    for (unsigned int i = 0; i < CIP_ConnectionManager::active_connections_set->size (); i++)
    {
        active_connection_manager = (CIP_ConnectionManager*)active_connections_set->at(i);

        if (CIP_Connection::GetInstanceNumber(active_connection_manager->producing_instance) == connection_id)
            return active_connection_manager;

        if (CIP_Connection::GetInstanceNumber(active_connection_manager->consuming_instance) == connection_id)
            return active_connection_manager;

    }
    return nullptr;
}

CIP_ConnectionManager *CIP_ConnectionManager::GetConnectedOutputAssembly (CipUdint output_assembly_id)
{
    CIP_ConnectionManager * active_connection_manager_instance;
    CIP_Connection * connection_instance;

    for (unsigned int i = 0; i < active_connections_set->size (); i++)
    {
        active_connection_manager_instance = (CIP_ConnectionManager*)active_connections_set->at(i);
        connection_instance = (CIP_Connection*) CIP_Connection::GetInstance (active_connection_manager_instance->connection_serial_number);

        if (connection_instance->State == CIP_Connection::kConnectionStateEstablished)
        {
            if (active_connection_manager_instance->connection_path.connection_point[0] == output_assembly_id)
                return active_connection_manager_instance;
        }
    }
    return nullptr;
}

CIP_ConnectionManager *CIP_ConnectionManager::CheckForExistingConnection (CIP_ConnectionManager * connection_manager_instance)
{
    CIP_ConnectionManager * active_connection_manager_instance;
    CIP_Connection * connection_instance;
    for (unsigned int i = 0; i < active_connections_set->size (); i++)
    {
        active_connection_manager_instance = (CIP_ConnectionManager*)active_connections_set->at(i);
        connection_instance = (CIP_Connection*) CIP_Connection::GetInstance (active_connection_manager_instance->connection_serial_number);

        if (connection_instance->State == CIP_Connection::kConnectionStateEstablished)
        {
            if ((connection_manager_instance->connection_serial_number == active_connection_manager_instance->connection_serial_number) &&
                (connection_manager_instance->originator_vendor_id == active_connection_manager_instance->originator_vendor_id) &&
                (connection_manager_instance->originator_serial_number == active_connection_manager_instance->originator_serial_number))
            {
                return active_connection_manager_instance;
            }
        }
    }
    return nullptr;
}

CipStatus CIP_ConnectionManager::CheckElectronicKeyData (CipUsint key_format, CIP_ElectronicKey *key_data, CipUint *extended_status)
{
    CipByte compatiblity_mode = (CipByte) (key_data->data.major_revision & 0x80);

    // Remove compatibility bit
    key_data->data.major_revision &= 0x7F;

    // Default return value
    *extended_status = kConnectionManagerStatusCodeSuccess;

    // Check key format
    if (4 != key_format)
    {
        *extended_status = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
        return kCipStatusError;
    }

    // Check VendorID and ProductCode, must match, or 0
    if (((key_data->data.vendor_id != CIP_Identity::vendor_id_) && (key_data->data.vendor_id != 0))
        || ((key_data->data.product_code != CIP_Identity::product_code_) && (key_data->data.product_code != 0)))
    {
        *extended_status = kConnectionManagerStatusCodeErrorVendorIdOrProductcodeError;
        return kCipStatusError;
    }


    // VendorID and ProductCode are correct

    // Check DeviceType, must match or 0
    if ((key_data->data.device_type != CIP_Identity::device_type_) && (key_data->data.device_type != 0))
    {
        *extended_status = kConnectionManagerStatusCodeErrorDeviceTypeError;
        return kCipStatusError;
    }


    // VendorID, ProductCode and DeviceType are correct

    if (!compatiblity_mode)
    {
        // Major = 0 is valid
        if (0 == key_data->data.major_revision)
        {
            return (kCipStatusOk);
        }

        // Check Major / Minor Revision, Major must match, Minor match or 0
        if ((key_data->data.major_revision != CIP_Identity::revision_.major_revision)
            || ((key_data->data.minor_revision != CIP_Identity::revision_.minor_revision) && (key_data->data.minor_revision != 0)))
        {
            *extended_status = kConnectionManagerStatusCodeErrorRevisionMismatch;
            return kCipStatusError;
        }
    }
    else
    {
        // Compatibility mode is set

        // Major must match, Minor != 0 and <= MinorRevision
        if ((key_data->data.major_revision == CIP_Identity::revision_.major_revision)
            && (key_data->data.minor_revision > 0) && (key_data->data.minor_revision <= CIP_Identity::revision_.minor_revision))
        {
            return (kCipStatusOk);
        }
        else
        {
            *extended_status = kConnectionManagerStatusCodeErrorRevisionMismatch;
            return kCipStatusError;
        }
    } // end if CompatiblityMode handling



    return (*extended_status == kConnectionManagerStatusCodeSuccess) ? kCipStatusOk : kCipStatusError;
}

CipUsint CIP_ConnectionManager::ParseConnectionPath (CipMessageRouterRequest_t *message_router_request, CipUint *extended_error)
{
    CipUsint *message = &message_router_request->request_data[0];
    int remaining_path_size = connection_path_size = *message++; // length in words

    int originator_to_target_connection_type;
    int target_to_originator_connection_type;

    // with 256 we mark that we haven't got a PIT segment
    production_inhibit_time = 256;

    if ((g_kForwardOpenHeaderLength + remaining_path_size * 2) < message_router_request->request_path_size)
    {
        // the received packet is larger than the data in the path
        *extended_error = 0;
        return kCipErrorTooMuchData;
    }
    
    if ((g_kForwardOpenHeaderLength + remaining_path_size * 2) > message_router_request->request_path_size)
    {
        //there is not enough data in received packet
        *extended_error = 0;
        return kCipErrorNotEnoughData;
    }

    if (remaining_path_size > 0)
    {
        // first electronic key
        if (*message == 0x34)
        {
            if (remaining_path_size < 5)
            {
                //there is not enough data for holding the electronic key segment
                *extended_error = 0;
                return kCipErrorNotEnoughData;
            }

            // logical electronic key found
            electronic_key.segment_header.bitfield_u.seg_type = CIP_Segment::segtype_logical_segment;
            electronic_key.segment_header.bitfield_u.logical_u.logical_format = CIP_Segment::logical_special;
            electronic_key.segment_header.bitfield_u.logical_u.logical_type = CIP_Segment::logical_8bitaddress;
            electronic_key.segment_payload.reserve(5); //electronic_key.segment_type = 0x34;
            message++;

            CIP_ElectronicKey * key = (CIP_ElectronicKey*)&electronic_key.segment_payload[0];
            key->key_format = *message++;

            key->data.vendor_id = NET_Endianconv::GetIntFromMessage (message);
            message += 2;

            key->data.device_type = NET_Endianconv::GetIntFromMessage (message);
            message += 2;

            key->data.product_code = NET_Endianconv::GetIntFromMessage (message);
            message += 2;

            key->data.major_revision = *message++;
            key->data.minor_revision = *message++;

            OPENER_TRACE_INFO("key: ven ID %d, dev type %d, prod code %d, major %d, minor %d\n",
                              connection_object->electronic_key.key_data.vendor_id,
                              connection_object->electronic_key.key_data.device_type,
                              connection_object->electronic_key.key_data.product_code,
                              connection_object->electronic_key.key_data.major_revision,
                              connection_object->electronic_key.key_data.minor_revision);

            if (kCipStatusOk != CheckElectronicKeyData (key->key_format, key, extended_error).status)
            {
                return kCipErrorConnectionFailure;
            }
        }
        else
        {
            OPENER_TRACE_INFO("no key\n");
        }

        if (producing_instance->TransportClass_trigger.bitfield_u.production_trigger
            != CIP_Connection::kConnectionTriggerProductionTriggerCyclic)
        {
            // non cyclic connections may have a production inhibit
            if (kProductionTimeInhibitTimeNetworkSegment == *message)
            {
                production_inhibit_time = message[1];
                message += 2;
                remaining_path_size -= 1;
            }
        }

        if (EQLOGICALPATH(*message, 0x20))
        {
            // classID
            connection_path.class_id = GetPaddedLogicalPath (message);
            class_ptr = this->class_ptr;
            if (0 == class_ptr)
            {
                OPENER_TRACE_ERR("classid %" PRIx32 " not found\n", connection_path.class_id);
                if (connection_path.class_id >= 0xC8)
                {
                    //reserved range of class ids
                    *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
                }
                else
                {
                    *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                }
                return kCipErrorConnectionFailure;
            }

            OPENER_TRACE_INFO("classid %" PRIx32 " (%s)\n", connection_path.class_id, class_ptr->class_name);
        }
        else
        {
            *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
            return kCipErrorConnectionFailure;
        }
        remaining_path_size -= 1; // 1 16Bit word for the class part of the path

        if (EQLOGICALPATH(*message, 0x24))
        {
            // store the configuration ID for later checking in the application connection types
            connection_path.connection_point[2] = GetPaddedLogicalPath (message);
            OPENER_TRACE_INFO("Configuration instance id %" PRId32 "\n", connection_path.connection_point[2]);
            //todo: fix this -> if (nullptr == connection_path.connection_point[2])
            {
                // according to the test tool we should respond with this extended error code
                *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
                return kCipErrorConnectionFailure;
            }
            // 1 or 2 16Bit words for the configuration instance part of the path
            //remaining_path_size -= (connection_path.connection_point[2] > 0xFF) ? 2 : 1;
        } else
        {
            OPENER_TRACE_INFO("no config data\n");
        }

        if (producing_instance->TransportClass_trigger.bitfield_u.transport_class
            == CIP_Connection::kConnectionTriggerTransportClass3)
        {
            //we have Class 3 connection
            if (remaining_path_size > 0)
            {
                OPENER_TRACE_WARN("Too much data in connection path for class 3 connection\n");
                *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
                return kCipErrorConnectionFailure;
            }

            // connection end point has to be the message router instance 1
            if ((connection_path.class_id != kCipMessageRouterClassCode) || (connection_path.connection_point[2] != 1))
            {
                *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                return kCipErrorConnectionFailure;
            }
            connection_path.connection_point[0] = connection_path.connection_point[2];
        }
        else
        {
             // we have an IO connection
            originator_to_target_connection_type = (o_to_t_network_connection_parameter & 0x6000) >> 13;
            target_to_originator_connection_type = (t_to_o_network_connection_parameter & 0x6000) >> 13;

            connection_path.connection_point[1] = 0; /* set not available path to Invalid */

            int number_of_encoded_paths = 0;
            if (originator_to_target_connection_type == 0)
            {
                if (target_to_originator_connection_type == 0)
                {
                    // configuration only connection
                    number_of_encoded_paths = 0;
                    OPENER_TRACE_WARN("assembly: type invalid\n");
                }
                else
                {
                    // 1 path -> path is for production
                    OPENER_TRACE_INFO("assembly: type produce\n");
                    number_of_encoded_paths = 1;
                }
            }
            else
            {
                if (target_to_originator_connection_type == 0)
                {
                    // 1 path -> path is for consumption
                    OPENER_TRACE_INFO("assembly: type consume\n");
                    number_of_encoded_paths = 1;
                } else
                {
                    // 2 paths -> 1st for production 2nd for consumption
                    OPENER_TRACE_INFO("assembly: type bidirectional\n");
                    number_of_encoded_paths = 2;
                }
            }

            for (int i = 0; i < number_of_encoded_paths; i++) // process up to 2 encoded paths
            {
                // Connection Point interpreted as InstanceNr -> only in Assembly Objects
                if (EQLOGICALPATH(*message, 0x24) || EQLOGICALPATH(*message, 0x2C))
                {
                    // InstanceNR
                    connection_path.connection_point[i] = GetPaddedLogicalPath (message);
                    OPENER_TRACE_INFO("connection point %" PRIu32 "\n", connection_path.connection_point[i]);
                    if (0 == connection_path.connection_point[i])
                    {
                        *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                        return kCipErrorConnectionFailure;
                    }
                    // 1 or 2 16Bit word for the connection point part of the path
                    remaining_path_size -= (connection_path.connection_point[i] > 0xFF) ? 2 : 1;
                } else
                {
                    *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
                    return kCipErrorConnectionFailure;
                }
            }

            CIP_ConnectionManager::g_config_data_length = 0;
            CIP_ConnectionManager::g_config_data_buffer = nullptr;

            while (remaining_path_size > 0)
            {
                // have something left in the path should be configuration data

                switch (*message)
                {
                    case kDataSegmentTypeSimpleDataMessage:
                        // we have a simple data segment
                        CIP_ConnectionManager::g_config_data_length = (unsigned int) (message[1] * 2); //data segments store length 16-bit word wise
                        CIP_ConnectionManager::g_config_data_buffer = &(message[2]);
                        remaining_path_size -= (CIP_ConnectionManager::g_config_data_length + 2);
                        message += (CIP_ConnectionManager::g_config_data_length + 2);
                        break;
                        // TODO do we have to handle ANSI extended symbol data segments too?
                    case kProductionTimeInhibitTimeNetworkSegment:
                        if (CIP_Connection::kConnectionTriggerProductionTriggerCyclic
                            != producing_instance->TransportClass_trigger.bitfield_u.production_trigger)
                        {
                            // only non cyclic connections may have a production inhibit
                            production_inhibit_time = message[1];
                            message += 2;
                            remaining_path_size -= 2;
                        }
                        else
                        {
                            //offset in 16Bit words where within the connection path the error happend
                            *extended_error = (CipUint) (connection_path_size - remaining_path_size);
                            //status code for invalid segment type
                            return kCipErrorPathSegmentError;
                        }
                        break;
                    default:
                        OPENER_TRACE_WARN("No data segment identifier found for the configuration data\n");
                        //offset in 16Bit words where within the connection path the error happend
                        *extended_error = (CipUint) (connection_path_size - remaining_path_size);

                        //status code for invalid segment type
                        return 0x04;
                        break;
                }
            }
        }
    }

    // save back the current position in the stream allowing followers to parse anything thats still there
    //message_router_request->request_data = message;
    return kCipStatusOk;
}

void CIP_ConnectionManager::CloseConnection (CIP_ConnectionManager * connection_manager_instance)
{
    CIP_Connection * connection_instance;
    connection_instance = (CIP_Connection*) CIP_Connection::GetInstance (connection_manager_instance->id);

    connection_instance->State = CIP_Connection::kConnectionStateNonExistent;
    if (connection_instance->TransportClass_trigger.bitfield_u.transport_class != CIP_Connection::kConnectionTriggerTransportClass3)
    {
        // only close the UDP connection for not class 3 connections
        //IApp_CloseSocket_udp (pa_pstConnObj->netConn->GetSocketHandle());
        //netConn->SetSocketHandle(kEipInvalidSocket);
        connection_instance->netConn->CloseSocket ();
    }
    //todo: close all bounded connections
    RemoveFromActiveConnections (connection_manager_instance);
}

void CIP_ConnectionManager::CopyConnectionData (CIP_Connection *pa_pstDst,const  CIP_Connection *pa_pstSrc)
{
    memcpy (pa_pstDst, pa_pstSrc, sizeof (CIP_Connection));
}

void CIP_ConnectionManager::AddNewActiveConnection (CIP_Connection * connection_object)
{
    connection_object->State = CIP_Connection::kConnectionStateEstablished;
}

void CIP_ConnectionManager::RemoveFromActiveConnections (CIP_ConnectionManager * connection_manager_instance)
{
    CIP_Connection * connection_instance;
    connection_instance = (CIP_Connection*) CIP_Connection::GetInstance (connection_manager_instance->connection_serial_number);
    connection_instance->State = CIP_Connection::kConnectionStateNonExistent;
    active_connections_set->erase ((const unsigned int &) connection_manager_instance->id);
}

CipBool CIP_ConnectionManager::IsConnectedOutputAssembly (CipUdint pa_nInstanceNr)
{
    CipBool bRetVal = (CipBool)false;

    CIP_ConnectionManager *pstRunner;

    for (unsigned int i = 0; i < active_connections_set->size (); i++)
    {
        pstRunner = (CIP_ConnectionManager*)active_connections_set->at(i);
        if (pa_nInstanceNr == pstRunner->connection_path.connection_point[0])
        {
            bRetVal = (CipBool)true;
            break;
        }
    }
    return bRetVal;
}


CipStatus CIP_ConnectionManager::TriggerConnections (CipUdint pa_unOutputAssembly, CipUdint pa_unInputAssembly)
{
    CipStatus nRetVal = kCipStatusError;

    CIP_ConnectionManager *pstRunner;

    for (int i = 0; i < active_connections_set->size (); i++)
    {
        pstRunner = (CIP_ConnectionManager*)active_connections_set->at(i);

        if ((pa_unOutputAssembly == pstRunner->connection_path.connection_point[0])
            && (pa_unInputAssembly == pstRunner->connection_path.connection_point[1]))
        {
            if (CIP_Connection::kConnectionTriggerProductionTriggerApplicationObj
                == pstRunner->producing_instance->TransportClass_trigger.bitfield_u.production_trigger)
            {
                // produce at the next allowed occurrence
                pstRunner->transmission_trigger_timer = pstRunner->production_inhibit_timer;
                nRetVal = kCipStatusOk;
            }
            break;
        }
    }
    return nRetVal;
}

CipStatus CIP_ConnectionManager::InstanceServices(int service,
                                                  CipMessageRouterRequest_t* message_router_request,
                                                  CipMessageRouterResponse_t* message_router_response)
{
    //Class services
    if (this->id == 0)
    {
        switch(service)
        {
            case kForwardOpen:
		//todo: fix return ForwardOpen(connection_object, message_router_request, message_router_response);
                break;
            case kForwardClose:
		return ForwardClose(message_router_request, message_router_response);
                break;
            case kGetConnectionOwner:
                return GetConnectionOwner(message_router_request, message_router_response);
                break;
            default:
              return kCipStatusError;
        }
    }
    //Instance services
    else
    {
	    return kCipStatusError;
    }
}

CipStatus CIP_ConnectionManager::Shut()
{
    CipStatus stat;
    stat.status = kCipStatusOk;
    return stat;
}