/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <string.h>

#include "CIP_IOConnection.h"
#include "CIP_MessageRouter.h"
#include "appcontype.h"
#include "src/cip/CIP_Assembly.h"
#include "CIP_Common.h"
#include "CIP_Connection.h"
#include "../network_stack/ethernetip_net/tcpip_link/ciptcpipinterface.h"
#include "endianconv.h"
#include "src/cip/network_stack/NET_NetworkHandler.h"
#include "trace.h"


CipStatus CIP_IOConnection::EstablishIoConnction(CipUint* extended_error)
{
    int originator_to_target_connection_type, target_to_originator_connection_type;
    CipStatus eip_status = kCipStatusOk;
    CIP_Attribute* attribute;

    // currently we allow I/O connections only to assembly objects
    // we don't need to check for zero as this is handled in the connection path parsing
    CIP_Assembly* assembly_class = CIP_Assembly::GetClass();
    CIP_Assembly* instance = NULL;

   GetIoConnectionForConnectionData(cipConn, extended_error);

    if (NULL == cipConn)
    {
        return (CipStatus) kCipErrorConnectionFailure;
    }

    // TODO: add check for transport type trigger

    if (CIP_Connection::kConnectionTriggerTypeCyclicConnection != (cipConn->transport_type_class_trigger & CIP_Connection::kConnectionTriggerTypeProductionTriggerMask))
    {
        if (256 == cipConn->production_inhibit_time)
        {
            //there was no PIT segment in the connection path set PIT to one fourth of RPI
            cipConn->production_inhibit_time = ((CipUint)(cipConn->t_to_o_requested_packet_interval) / 4000);
        }
        else
        {
            // if production inhibit time has been provided it needs to be smaller than the RPI
            if (cipConn->production_inhibit_time > ((CipUint)((cipConn->t_to_o_requested_packet_interval) / 1000)))
            {
                // see section C-1.4.3.3
                *extended_error = 0x111; //< RPI not supported. Extended Error code deprecated
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }
    }
    // set the connection call backs
    //cipConn->connection_close_function = CloseIoConnection;
    //cipConn->connection_timeout_function = HandleIoConnectionTimeOut;
    //cipConn->connection_send_data_function = SendConnectedData;
    //cipConn->connection_receive_data_function = HandleReceivedIoConnectionData;

    cipConn->GeneralConnectionConfiguration ();

    originator_to_target_connection_type = (cipConn->o_to_t_network_connection_parameter & 0x6000) >> 13;
    target_to_originator_connection_type = (cipConn->t_to_o_network_connection_parameter & 0x6000) >> 13;

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

        cipConn->consuming_instance = 0;
        cipConn->consumed_connection_path_length = 0;
        cipConn->producing_instance = 0;
        cipConn->produced_connection_path_length = 0;

        if (originator_to_target_connection_type != 0)
        {
            //setup consumer side
            if (0 != (instance = CIP_Assembly::GetInstance (cipConn->connection_path.connection_point[0])))
            {
                // consuming Connection Point is present
                cipConn->consuming_instance = instance;

                cipConn->consumed_connection_path_length = 6;
                cipConn->consumed_connection_path.path_size = 6;
                cipConn->consumed_connection_path.class_id = cipConn->connection_path.class_id;
                cipConn->consumed_connection_path.instance_number = cipConn->connection_path.connection_point[0];
                cipConn->consumed_connection_path.attribute_number = 3;

                attribute = instance->GetCipAttribute (3);
                OPENER_ASSERT(attribute != NULL);
                // an assembly object should always have an attribute 3
                data_size = cipConn->consumed_connection_size;
                diff_size = 0;
                is_heartbeat = (((CipByteArray*)attribute->getData ())->length == 0);

                if ((cipConn->transport_type_class_trigger & 0x0F) == 1)
                {
                    // class 1 connection
                    data_size -= 2; // remove 16-bit sequence count length
                    diff_size += 2;
                }

                if ((kOpenerConsumedDataHasRunIdleHeader) && (data_size > 0) && (!is_heartbeat))
                {
                    // we only have an run idle header if it is not an heartbeat connection
                    data_size -= 4; // remove the 4 bytes needed for run/idle header
                    diff_size += 4;
                }

                if (((CipByteArray*)attribute->getData())->length != data_size)
                {
                    /*wrong connection size */
                    cipConn->correct_originator_to_target_size = ((CipByteArray*)attribute->getData())->length + diff_size;
                    *extended_error = CIP_Connection::kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize;
                    return (CipStatus) kCipErrorConnectionFailure;
                }
            }
            else
            {
                *extended_error = CIP_Connection::kConnectionManagerStatusCodeInvalidConsumingApllicationPath;
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        if (target_to_originator_connection_type != 0)
        {
            //setup producer side
            if (0 != (instance = CIP_Assembly::GetInstance(cipConn->connection_path.connection_point[producing_index])))
            {
                cipConn->producing_instance = instance;

                cipConn->produced_connection_path_length = 6;
                cipConn->produced_connection_path.path_size = 6;
                cipConn->produced_connection_path.class_id = cipConn->connection_path.class_id;
                cipConn->produced_connection_path.instance_number = cipConn->connection_path.connection_point[producing_index];
                cipConn->produced_connection_path.attribute_number = 3;

                attribute = instance->GetCipAttribute (3);
                OPENER_ASSERT(attribute != NULL);
                // an assembly object should always have an attribute 3
                data_size = cipConn->produced_connection_size;
                diff_size = 0;
                is_heartbeat = (((CipByteArray*)attribute->getData())->length == 0);

                if ((cipConn->transport_type_class_trigger & 0x0F) == 1)
                {
                    // class 1 connection
                    data_size -= 2; //remove 16-bit sequence count length
                    diff_size += 2;
                }

                if ((kOpenerProducedDataHasRunIdleHeader) && (data_size > 0) && (!is_heartbeat))
                {
                    // we only have an run idle header if it is not an heartbeat connection
                    data_size -= 4; // remove the 4 bytes needed for run/idle header
                    diff_size += 4;
                }

                if (((CipByteArray*)attribute->getData())->length != data_size)
                {
                    /*wrong connection size*/
                    cipConn->correct_target_to_originator_size = ((CipByteArray*)attribute->getData())->length + diff_size;
                    *extended_error = CIP_Connection::kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize;
                    return (CipStatus) kCipErrorConnectionFailure;
                }

            }
            else
            {
                *extended_error = CIP_Connection::kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        if (NULL != g_config_data_buffer)
        {
            // config data has been sent with this forward open request
            *extended_error = HandleConfigData(assembly_class);
            if (0 != *extended_error)
            {
                return (CipStatus) kCipErrorConnectionFailure;
            }
        }

        eip_status = OpenCommunicationChannels(cipConn);
        if (kCipStatusOk != eip_status)
        {
            *extended_error = 0; //TODO find out the correct extended error code
            return eip_status;
        }
    }

    CIP_Connection::AddNewActiveConnection(cipConn);
    //todo:CheckIoConnectionEvent(cipConn->connection_path.connection_point[0], cipConn->connection_path.connection_point[1], kIoConnectionEventOpened);
    return eip_status;
}

/*   @brief Open a Point2Point connection dependent on pa_direction.
 *   @param cipConn Pointer to registered Object in ConnectionManager.
 *   @param cpf_data Index of the connection object
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus CIP_IOConnection::OpenConsumingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    //static EIP_UINT16 nUDPPort = 2222; //TODO think on improving the udp port assigment for point to point connections 
    int j = 0;
    struct sockaddr_in addr;
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

    ///todo: create UDP socket only with CIP_Connection
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    //addr.in_port = htons(nUDPPort++);
    addr.sin_port = htons(kOpenerEipIoUdpPort);

    socket = CreateUdpSocket(kUdpCommuncationDirectionConsuming, &addr); // the address is only needed for bind used if consuming
    if (socket == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenPointToPointConnection\n");
        return kCipStatusError;
    }

    cipConn->originator_address = addr; // store the address of the originator for packet scanning
    addr.sin_addr.s_addr = INADDR_ANY; // restore the address
    cipConn->netConn->SetSocketHandle (socket);

    cpf_data->address_info_item[j].length = 16;
    cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget;

    cpf_data->address_info_item[j].sin_port = addr.sin_port;
    //TODO should we add our own address here?
    cpf_data->address_info_item[j].sin_addr = addr.sin_addr.s_addr;
    memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
    cpf_data->address_info_item[j].sin_family = htons(AF_INET);

    return kCipStatusOk;
}

CipStatus CIP_IOConnection::OpenProducingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    int socket;
    in_port_t port = htons(kOpenerEipIoUdpPort); /* the default port to be used if no port information is part of the forward open request */

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

    cipConn->remote_address.sin_family = AF_INET;
    cipConn->remote_address.sin_addr.s_addr = 0; /* we don't know the address of the originate will be set in the IApp_CreateUDPSocket */
    cipConn->remote_address.sin_port = port;

    socket = CreateUdpSocket(kUdpCommuncationDirectionProducing, &cipConn->netConn->remote_address); /* the address is only needed for bind used if consuming */

    if (socket == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenPointToPointConnection\n");
        // *pa_pnExtendedError = 0x0315; //miscellaneous
        return (CipStatus) kCipErrorConnectionFailure;
    }
    cipConn->netConn->SetSocketHandle (socket);

    return kCipStatusOk;
}

CipStatus CIP_IOConnection::OpenProducingMulticastConnection(CIP_CommonPacket::PacketFormat* cpf_data)
{
    CIP_Connection* existing_cipConn = GetExistingProducerMulticastConnection(cipConn->connection_path.connection_point[1]);
    int j;

    if (NULL == existing_cipConn)
    {
        // we are the first connection producing for the given Input Assembly
        return OpenMulticastConnection(kUdpCommuncationDirectionProducing, cpf_data);
    }
    else
    {
        // we need to inform our originator on the correct connection id
        cipConn->produced_connection_id = existing_cipConn->produced_connection_id;
    }

    // we have a connection reuse the data and the socket

    // allocate an unused sockaddr struct to use
    j = 0;

    if (g_cpf_data_item.address_info_item[0].type_id == 0)
    {
        // it is not used yet
        j = 0;
    }
    else if (g_cpf_data_item.address_info_item[1].type_id == 0)
    {
        j = 1;
    }

    if (CIP_Connection::kConnectionTypeIoExclusiveOwner == cipConn->instance_type)
    {
        //exclusive owners take the socket and further manage the connection
        //especially in the case of time outs.

        cipConn->netConn->SetSocketHandle (existing_cipConn->netConn->GetSocketHandle ());
        existing_cipConn->netConn->SetSocketHandle (kEipInvalidSocket);
    }
    else
    {
        // this connection will not produce the data
        cipConn->netConn->SetSocketHandle (kEipInvalidSocket);
    }

    cpf_data->address_info_item[j].length = 16;
    cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator;
    cipConn->netConn->remote_address.sin_family = AF_INET;
    cipConn->netConn->remote_address.sin_port = cpf_data->address_info_item[j].sin_port = htons(kOpenerEipIoUdpPort);
    cipConn->netConn->remote_address.sin_addr.s_addr = cpf_data->address_info_item[j].sin_addr = g_multicast_configuration.starting_multicast_address;
    memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
    cpf_data->address_info_item[j].sin_family = htons(AF_INET);

    return kCipStatusOk;
}

/**  @brief Open a Multicast connection dependent on @var direction.
 *
 *   @param direction Flag to indicate if consuming or producing.
 *   @param cipConn  pointer to registered Object in ConnectionManager.
 *   @param cpf_data     received CPF Data Item.
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus CIP_IOConnection::OpenMulticastConnection(UdpCommuncationDirection direction, CIP_CommonPacket::PacketFormat* cpf_data)
{
    int j = 0;
    int socket;

    if (0 != g_cpf_data_item.address_info_item[0].type_id) {
        if ((kUdpCommuncationDirectionConsuming == direction) && (CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget == cpf_data->address_info_item[0].type_id))
        {
            // for consuming connection points the originator can choose the multicast address to use
            // we have a given address type so use it
        }
        else
        {
            j = 1;
            /* if the type is not zero (not used) or if a given type it has to be the correct one */
            if ((0 != g_cpf_data_item.address_info_item[1].type_id) && (!((kUdpCommuncationDirectionConsuming == direction)
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
        cpf_data->address_info_item[j].sin_family = htons(AF_INET);
        cpf_data->address_info_item[j].sin_port = htons(kOpenerEipIoUdpPort);
        cpf_data->address_info_item[j].sin_addr = g_multicast_configuration.starting_multicast_address;
        memset(cpf_data->address_info_item[j].nasin_zero, 0, 8);
        cpf_data->address_info_item[j].length = 16;
    }

    if (htons(AF_INET) != cpf_data->address_info_item[j].sin_family)
    {
        OPENER_TRACE_ERR("Sockaddr Info Item with wrong sin family value recieved\n");
        return kCipStatusError;
    }

    /* allocate an unused sockaddr struct to use */
    struct sockaddr_in socket_address;
    socket_address.sin_family = ntohs(cpf_data->address_info_item[j].sin_family);
    socket_address.sin_addr.s_addr = cpf_data->address_info_item[j].sin_addr;
    socket_address.sin_port = cpf_data->address_info_item[j].sin_port;

    // the address is only needed for bind used if consuming
    socket = CreateUdpSocket(direction, &socket_address);
    if (socket == kEipInvalidSocket)
    {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenMulticastConnection\n");
        return kCipStatusError;
    }
    cipConn->socket[direction] = socket;

    if (direction == kUdpCommuncationDirectionConsuming)
    {
        cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoOriginatorToTarget;
        cipConn->originator_address = socket_address;
    }
    else
    {
        cpf_data->address_info_item[j].type_id = CIP_CommonPacket::kCipItemIdSocketAddressInfoTargetToOriginator;
        cipConn->remote_address = socket_address;
    }

    return kCipStatusOk;
}

CipUint CIP_IOConnection::HandleConfigData(CIP_ClassInstance* assembly_class)
{
    CipUint connection_manager_status = 0;
    CIP_ClassInstance* config_instance = GetCIPClass(assembly_class, cipConn->connection_path.connection_point[2]);

    if (0 != g_config_data_length)
    {
        if (ConnectionWithSameConfigPointExists(cipConn->connection_path.connection_point[2]))
        {
            // there is a connected connection with the same config point
            // we have to have the same data as already present in the config point
            CipByteArray* p = (CipByteArray*)GetCipAttribute(config_instance, 3)->data;
            if (p->length != g_config_data_length)
            {
                connection_manager_status = CIP_Connection::kConnectionManagerStatusCodeErrorOwnershipConflict;
            }
            else
            {
                /*FIXME check if this is correct */
                if (memcmp(p->data, g_config_data_buffer, g_config_data_length))
                {
                    connection_manager_status = CIP_Connection::kConnectionManagerStatusCodeErrorOwnershipConflict;
                }
            }
        }
        else
        {
            // put the data on the configuration assembly object with the current
            // design this can be done rather efficiently
            if (kCipStatusOk != NotifyAssemblyConnectedDataReceived(config_instance, g_config_data_buffer, g_config_data_length))
            {
                OPENER_TRACE_WARN("Configuration data was invalid\n");
                connection_manager_status = CIP_Connection::kConnectionManagerStatusCodeInvalidConfigurationApplicationPath;
            }
        }
    }
    return connection_manager_status;
}

void CIP_IOConnection::CloseIoConnection()
{

    CheckIoConnectionEvent(cipConn->connection_path.connection_point[0], cipConn->connection_path.connection_point[1], kIoConnectionEventClosed);

    if ((CIP_Connection::kConnectionTypeIoExclusiveOwner == cipConn->instance_type) || (CIP_Connection::kConnectionTypeIoInputOnly == cipConn->instance_type))
    {
        if ((CIP_Connection::kRoutingTypeMulticastConnection == (cipConn->t_to_o_network_connection_parameter & CIP_Connection::kRoutingTypeMulticastConnection)) && (kEipInvalidSocket != cipConn->sock))
        {
            CIP_Connection* next_non_control_master_connection = GetNextNonControlMasterConnection(cipConn->connection_path.connection_point[1]);
            if (NULL != next_non_control_master_connection)
            {
                next_non_control_master_connection->netConn->SetSocketHandle (cipConn->netConn->GetSocketHandle ());

                memcpy(&(next_non_control_master_connection->netConn->remote_address), &(cipConn->netConn->remote_address), sizeof(next_non_control_master_connection->remote_address));

                next_non_control_master_connection->eip_level_sequence_count_producing = cipConn->eip_level_sequence_count_producing;

                next_non_control_master_connection->sequence_count_producing = cipConn->sequence_count_producing;
                cipConn->netConn->SetSocketHandle (kEipInvalidSocket);
                next_non_control_master_connection->transmission_trigger_timer = cipConn->transmission_trigger_timer;
            }
            else
            {
                // this was the last master connection close all listen only connections listening on the port
                CloseAllConnectionsForInputWithSameType(cipConn->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
            }
        }
    }

    CloseCommunicationChannelAndRemoveFromActiveConnectionsList(cipConn);
}

void CIP_IOConnection::HandleIoConnectionTimeOut()
{
    CIP_Connection* next_non_control_master_connection;
    CheckIoConnectionEvent(cipConn->connection_path.connection_point[0], cipConn->connection_path.connection_point[1], kIoConnectionEventTimedOut);

    if (CIP_Connection::kRoutingTypeMulticastConnection == (cipConn->t_to_o_network_connection_parameter & CIP_Connection::kRoutingTypeMulticastConnection))
    {
        switch (cipConn->instance_type)
        {
            case CIP_Connection::kConnectionTypeIoExclusiveOwner:
                CloseAllConnectionsForInputWithSameType(cipConn->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoInputOnly);
                CloseAllConnectionsForInputWithSameType(cipConn->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
                break;
            case CIP_Connection::kConnectionTypeIoInputOnly:
                if (kEipInvalidSocket != cipConn->netConn->GetSocketHandle ())
                {
                    // we are the controlling input only connection find a new controller
                    next_non_control_master_connection = GetNextNonControlMasterConnection(cipConn->connection_path.connection_point[1]);
                    if (NULL != next_non_control_master_connection)
                    {
                        next_non_control_master_connection->conn->SetSocketHandle (cipConn->netConn->GetSocketHandle ());
                        cipConn->netConn->SetSocketHandle (kEipInvalidSocket);
                        next_non_control_master_connection->transmission_trigger_timer = cipConn->transmission_trigger_timer;
                    }
                    else
                    {
                        //this was the last master connection close all listen only connections listening on the port
                        CloseAllConnectionsForInputWithSameType(cipConn->connection_path.connection_point[1], CIP_Connection::kConnectionTypeIoListenOnly);
                    }
                }
                break;
            default:
                break;
        }
    }

    OPENER_ASSERT(NULL != cipConn->CloseConnection (cipConn));
    cipConn->CloseConnection(cipConn);
}

CipStatus CIP_IOConnection::SendConnectedData(CIP_Connection* cipConn)
{
    CIP_CommonPacket::PacketFormat* cpf_data;
    CipUint reply_length;
    CipUsint* message_data_reply_buffer;

    /* TODO think of adding an own send buffer to each connection object in order to preset up the whole message on connection opening and just change the variable data items e.g., sequence number */

    cpf_data = &g_cpf_data_item; /* TODO think on adding a CPF data item to the S_CIP_CIP_Connection in order to remove the code here or even better allocate memory in the connection object for storing the message to send and just change the application data*/

    cipConn->eip_level_sequence_count_producing++;

    /* assembleCPFData */
    cpf_data->item_count = 2;
    if ((cipConn->transport_type_class_trigger & 0x0F) != 0) 
    { 
        // use Sequenced Address Items if not Connection Class 0 
        cpf_data->address_item.type_id = CIP_CommonPacket::kCipItemIdSequencedAddressItem;
        cpf_data->address_item.length = 8;
        cpf_data->address_item.data.sequence_number = cipConn->eip_level_sequence_count_producing;
    }
    else 
    {
        cpf_data->address_item.type_id = CIP_CommonPacket::kCipItemIdConnectionAddress;
        cpf_data->address_item.length = 4;
    }
    cpf_data->address_item.data.connection_identifier = cipConn->produced_connection_id;

    cpf_data->data_item.type_id = CIP_CommonPacket::kCipItemIdConnectedDataItem;

    CipByteArray* producing_instance_attributes = (CipByteArray*)cipConn->producing_instance->attributes->data;
    cpf_data->data_item.length = 0;

    // notify the application that data will be sent immediately after the call
    if (BeforeAssemblyDataSend(cipConn->producing_instance))
    {
        // the data has changed increase sequence counter
        cipConn->sequence_count_producing++;
    }

    // set AddressInfo Items to invalid Type
    cpf_data->address_info_item[0].type_id = 0;
    cpf_data->address_info_item[1].type_id = 0;

    reply_length = AssembleIOMessage(cpf_data, &g_message_data_reply_buffer[0]);

    message_data_reply_buffer = &g_message_data_reply_buffer[reply_length - 2];
    cpf_data->data_item.length = producing_instance_attributes->length;

    if (kOpenerProducedDataHasRunIdleHeader)
    {
        cpf_data->data_item.length += 4;
    }

    if ((cipConn->transport_type_class_trigger & 0x0F) == 1)
    {
        cpf_data->data_item.length += 2;
        AddIntToMessage(cpf_data->data_item.length, &message_data_reply_buffer);
        AddIntToMessage(cipConn->sequence_count_producing, &message_data_reply_buffer);
    }
    else
    {
        AddIntToMessage(cpf_data->data_item.length, &message_data_reply_buffer);
    }

    if (kOpenerProducedDataHasRunIdleHeader)
    {
        AddDintToMessage(g_run_idle_state, &(message_data_reply_buffer));
    }

    memcpy(message_data_reply_buffer, producing_instance_attributes->data, producing_instance_attributes->length);

    reply_length += cpf_data->data_item.length;

    return SendUdpData(&cipConn->remote_address, cipConn->sock, &g_message_data_reply_buffer[0], reply_length);
}

CipStatus CIP_IOConnection::HandleReceivedIoConnectionData(CIP_Connection* cipConn, CipUsint* data, CipUint data_length)
{

    /* check class 1 sequence number*/
    if ((cipConn->transport_type_class_trigger & 0x0F) == 1)
    {
        CipUint sequence_buffer = GetIntFromMessage(&(data));
        if (SEQ_LEQ16(sequence_buffer, cipConn->sequence_count_consuming))
        {
            return kCipStatusOk; // no new data for the assembly
        }
        cipConn->sequence_count_consuming = sequence_buffer;
        data_length -= 2;
    }

    if (data_length > 0) {
        // we have no heartbeat connection
        if (kOpenerConsumedDataHasRunIdleHeader)
        {
            CipUdint nRunIdleBuf = GetDintFromMessage(&(data));
            if (g_run_idle_state != nRunIdleBuf)
            {
                RunIdleChanged(nRunIdleBuf);
            }
            g_run_idle_state = nRunIdleBuf;
            data_length -= 4;
        }

        if (NotifyAssemblyConnectedDataReceived(cipConn->consuming_instance, data, data_length) != 0)
        {
            return kCipStatusError;
        }
    }
    return kCipStatusOk;
}

CipStatus CIP_IOConnection::OpenCommunicationChannels(CIP_Connection* cipConn)
{

    CipStatus eip_status = kCipStatusOk;
    //get pointer to the CPF data, currently we have just one global instance of the struct. This may change in the future
    CIP_CommonPacket::PacketFormat* cpf_data = &g_cpf_data_item;

    int originator_to_target_connection_type = (cipConn->o_to_t_network_connection_parameter & 0x6000) >> 13;

    int target_to_originator_connection_type = (cipConn->t_to_o_network_connection_parameter & 0x6000) >> 13;

    // open a connection "point to point" or "multicast" based on the ConnectionParameter
    if (originator_to_target_connection_type == 1) //TODO: Fix magic number; Multicast consuming
    {
        if (OpenMulticastConnection(kUdpCommuncationDirectionConsuming, cipConn, cpf_data) == kCipStatusError)
        {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    } else if (originator_to_target_connection_type == 2) // TODO: Fix magic number; Point to Point consuming
    {
        if (OpenConsumingPointToPointConnection(cipConn, cpf_data) == kCipStatusError)
        {
            OPENER_TRACE_ERR("error in PointToPoint consuming connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }

    if (target_to_originator_connection_type == 1) // TODO: Fix magic number; Multicast producing
    {
        if (OpenProducingMulticastConnection(cipConn, cpf_data) == kCipStatusError)
        {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }
    else if (target_to_originator_connection_type == 2) // TODO: Fix magic number; Point to Point producing
    {

        if (OpenProducingPointToPointConnection(cipConn, cpf_data) != kCipStatusOk)
        {
            OPENER_TRACE_ERR("error in PointToPoint producing connection\n");
            return (CipStatus) kCipErrorConnectionFailure;
        }
    }

    return eip_status;
}

void CIP_IOConnection::CloseCommunicationChannelAndRemoveFromActiveConnectionsList()
{
    cipConn->netConn->CloseSocket ();

    CIP_Connection::RemoveFromActiveConnections(cipConn);
}
