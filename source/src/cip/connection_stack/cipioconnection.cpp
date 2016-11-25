/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <string.h>

#include "cipioconnection.h"

#include "appcontype.h"
#include "src/cip/CIP_Assembly.h"
#include "cipcommon.h"
#include "CIP_Connection.h"
#include "../network_stack/ethernetip_net/tcpip_link/ciptcpipinterface.h"
#include "CIP_CommonPacket.h"
#include "endianconv.h"
#include "generic_networkhandler.h"
#include "trace.h"

/*The port to be used per default for I/O messages on UDP.*/
const int kOpenerEipIoUdpPort = 0x08AE;

/* producing multicast connection have to consider the rules that apply for
 * application connection types.
 */
CipStatus OpenProducingMulticastConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data);

CipStatus OpenMulticastConnection(
    UdpCommuncationDirection direction, CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data);

CipStatus OpenConsumingPointToPointConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data);

CipStatus OpenProducingPointToPointConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data);

CipUint HandleConfigData(CIP_ClassInstance* assembly_class,
    CIP_Connection* connection_object);

/* Regularly close the IO connection. If it is an exclusive owner or input only
 * connection and in charge of the connection a new owner will be searched
 */
void CloseIoConnection(CIP_Connection* connection_object);

void HandleIoConnectionTimeOut(CIP_Connection* connection_object);

/** @brief  Send the data from the produced CIP Object of the connection via the socket of the connection object
 *   on UDP.
 *      @param connection_object  pointer to the connection object
 *      @return status  EIP_OK .. success
 *                     EIP_ERROR .. error
 */
CipStatus SendConnectedData(CIP_Connection* connection_object);

CipStatus HandleReceivedIoConnectionData(CIP_Connection* connection_object,
    CipUsint* data, CipUint data_length);

/**** Global variables ****/
CipUsint* g_config_data_buffer = NULL; /**< buffers for the config data coming with a forward open request. */
unsigned int g_config_data_length = 0;

CipUdint g_run_idle_state; /**< buffer for holding the run idle information. */

/**** Implementation ****/
CipStatus EstablishIoConnction(CIP_Connection* connection_object,
    CipUint* extended_error)
{
    int originator_to_target_connection_type,
        target_to_originator_connection_type;
    CipStatus eip_status = kCipStatusOk;
    CIP_Attribute* attribute;
    /* currently we allow I/O connections only to assembly objects */
    CIP_ClassInstance* assembly_class = GetCIPClass(kCipAssemblyClassCode); /* we don't need to check for zero as this is handled in the connection path parsing */
    CIP_ClassInstance* instance = NULL;

    CIP_Connection* io_connection_object = GetIoConnectionForConnectionData(
        connection_object, extended_error);

    if (NULL == io_connection_object) {
        return kCipErrorConnectionFailure;
    }

    /* TODO add check for transport type trigger */

    if (kConnectionTriggerTypeCyclicConnection
        != (io_connection_object->transport_type_class_trigger
               & kConnectionTriggerTypeProductionTriggerMask)) {
        if (256 == io_connection_object->production_inhibit_time) {
            /* there was no PIT segment in the connection path set PIT to one fourth of RPI */
            io_connection_object->production_inhibit_time = ((CipUint)(io_connection_object->t_to_o_requested_packet_interval)
                / 4000);
        } else {
            /* if production inhibit time has been provided it needs to be smaller than the RPI */
            if (io_connection_object->production_inhibit_time
                > ((CipUint)((io_connection_object
                                       ->t_to_o_requested_packet_interval)
                      / 1000))) {
                /* see section C-1.4.3.3 */
                *extended_error = 0x111; /**< RPI not supported. Extended Error code deprecated */
                return kCipErrorConnectionFailure;
            }
        }
    }
    /* set the connection call backs */
    io_connection_object->connection_close_function = CloseIoConnection;
    io_connection_object->connection_timeout_function = HandleIoConnectionTimeOut;
    io_connection_object->connection_send_data_function = SendConnectedData;
    io_connection_object->connection_receive_data_function = HandleReceivedIoConnectionData;

    GeneralConnectionConfiguration(io_connection_object);

    originator_to_target_connection_type = (io_connection_object->o_to_t_network_connection_parameter & 0x6000) >> 13;
    target_to_originator_connection_type = (io_connection_object->t_to_o_network_connection_parameter & 0x6000) >> 13;

    if ((originator_to_target_connection_type == 0) && (target_to_originator_connection_type == 0))
    { /* this indicates an re-configuration of the connection currently not supported and we should not come here as this is handled in the forwardopen function*/

    }
    else
    {
        int producing_index = 0;
        int data_size;
        int diff_size;
        int is_heartbeat;

        if ((originator_to_target_connection_type != 0)
            && (target_to_originator_connection_type != 0)) { /* we have a producing and consuming connection*/
            producing_index = 1;
        }

        io_connection_object->consuming_instance = 0;
        io_connection_object->consumed_connection_path_length = 0;
        io_connection_object->producing_instance = 0;
        io_connection_object->produced_connection_path_length = 0;

        if (originator_to_target_connection_type != 0) { /*setup consumer side*/
            if (0
                != (instance = GetCIPClass(
                        assembly_class,
                        io_connection_object->connection_path.connection_point[0]))) { /* consuming Connection Point is present */
                io_connection_object->consuming_instance = instance;

                io_connection_object->consumed_connection_path_length = 6;
                io_connection_object->consumed_connection_path.path_size = 6;
                io_connection_object->consumed_connection_path.class_id = io_connection_object->connection_path.class_id;
                io_connection_object->consumed_connection_path.instance_number = io_connection_object->connection_path.connection_point[0];
                io_connection_object->consumed_connection_path.attribute_number = 3;

                attribute = GetCipAttribute(instance, 3);
                OPENER_ASSERT(attribute != NULL);
                /* an assembly object should always have an attribute 3 */
                data_size = io_connection_object->consumed_connection_size;
                diff_size = 0;
                is_heartbeat = (((CipByteArray*)attribute->data)->length == 0);

                if ((io_connection_object->transport_type_class_trigger & 0x0F) == 1) {
                    /* class 1 connection */
                    data_size -= 2; /* remove 16-bit sequence count length */
                    diff_size += 2;
                }
                if ((kOpenerConsumedDataHasRunIdleHeader) && (data_size > 0)
                    && (!is_heartbeat)) { /* we only have an run idle header if it is not an heartbeat connection */
                    data_size -= 4; /* remove the 4 bytes needed for run/idle header */
                    diff_size += 4;
                }
                if (((CipByteArray*)attribute->data)->length != data_size) {
                    /*wrong connection size */
                    connection_object->correct_originator_to_target_size = ((CipByteArray*)attribute->data)->length + diff_size;
                    *extended_error = kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize;
                    return kCipErrorConnectionFailure;
                }
            } else {
                *extended_error = kConnectionManagerStatusCodeInvalidConsumingApllicationPath;
                return kCipErrorConnectionFailure;
            }
        }

        if (target_to_originator_connection_type != 0) { /*setup producer side*/
            if (0
                != (instance = GetCIPClass(
                        assembly_class,
                        io_connection_object->connection_path.connection_point[producing_index]))) {
                io_connection_object->producing_instance = instance;

                io_connection_object->produced_connection_path_length = 6;
                io_connection_object->produced_connection_path.path_size = 6;
                io_connection_object->produced_connection_path.class_id = io_connection_object->connection_path.class_id;
                io_connection_object->produced_connection_path.instance_number = io_connection_object->connection_path.connection_point[producing_index];
                io_connection_object->produced_connection_path.attribute_number = 3;

                attribute = GetCipAttribute(instance, 3);
                OPENER_ASSERT(attribute != NULL);
                /* an assembly object should always have an attribute 3 */
                data_size = io_connection_object->produced_connection_size;
                diff_size = 0;
                is_heartbeat = (((CipByteArray*)attribute->data)->length == 0);

                if ((io_connection_object->transport_type_class_trigger & 0x0F) == 1) {
                    /* class 1 connection */
                    data_size -= 2; /* remove 16-bit sequence count length */
                    diff_size += 2;
                }
                if ((kOpenerProducedDataHasRunIdleHeader) && (data_size > 0)
                    && (!is_heartbeat)) { /* we only have an run idle header if it is not an heartbeat connection */
                    data_size -= 4; /* remove the 4 bytes needed for run/idle header */
                    diff_size += 4;
                }
                if (((CipByteArray*)attribute->data)->length != data_size) {
                    /*wrong connection size*/
                    connection_object->correct_target_to_originator_size = ((CipByteArray*)attribute->data)->length + diff_size;
                    *extended_error = kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize;
                    return kCipErrorConnectionFailure;
                }

            } else {
                *extended_error = kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                return kCipErrorConnectionFailure;
            }
        }

        if (NULL != g_config_data_buffer) { /* config data has been sent with this forward open request */
            *extended_error = HandleConfigData(assembly_class, io_connection_object);
            if (0 != *extended_error) {
                return kCipErrorConnectionFailure;
            }
        }

        eip_status = OpenCommunicationChannels(io_connection_object);
        if (kCipStatusOk != eip_status) {
            *extended_error = 0; /*TODO find out the correct extended error code*/
            return eip_status;
        }
    }

    AddNewActiveConnection(io_connection_object);
    CheckIoConnectionEvent(io_connection_object->connection_path.connection_point[0],
        io_connection_object->connection_path.connection_point[1],
        kIoConnectionEventOpened);
    return eip_status;
}

/*   @brief Open a Point2Point connection dependent on pa_direction.
 *   @param connection_object Pointer to registered Object in ConnectionManager.
 *   @param common_packet_format_data Index of the connection object
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus OpenConsumingPointToPointConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data)
{
    /*static EIP_UINT16 nUDPPort = 2222; TODO think on improving the udp port assigment for point to point connections */
    int j = 0;
    struct sockaddr_in addr;
    int socket;

    if (common_packet_format_data->address_info_item[0].type_id == 0) { /* it is not used yet */
        j = 0;
    } else if (common_packet_format_data->address_info_item[1].type_id == 0) {
        j = 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    /*addr.in_port = htons(nUDPPort++);*/
    addr.sin_port = htons(kOpenerEipIoUdpPort);

    socket = CreateUdpSocket(kUdpCommuncationDirectionConsuming, &addr); /* the address is only needed for bind used if consuming */
    if (socket == kEipInvalidSocket) {
        OPENER_TRACE_ERR(
            "cannot create UDP socket in OpenPointToPointConnection\n");
        return kCipStatusError;
    }

    connection_object->originator_address = addr; /* store the address of the originator for packet scanning */
    addr.sin_addr.s_addr = INADDR_ANY; /* restore the address */
    connection_object->socket[kUdpCommuncationDirectionConsuming] = socket;

    common_packet_format_data->address_info_item[j].length = 16;
    common_packet_format_data->address_info_item[j].type_id = kCipItemIdSocketAddressInfoOriginatorToTarget;

    common_packet_format_data->address_info_item[j].sin_port = addr.sin_port;
    /*TODO should we add our own address here? */
    common_packet_format_data->address_info_item[j].sin_addr = addr.sin_addr
                                                                   .s_addr;
    memset(common_packet_format_data->address_info_item[j].nasin_zero, 0, 8);
    common_packet_format_data->address_info_item[j].sin_family = htons(AF_INET);

    return kCipStatusOk;
}

CipStatus OpenProducingPointToPointConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data)
{
    int socket;
    in_port_t port = htons(kOpenerEipIoUdpPort); /* the default port to be used if no port information is part of the forward open request */

    if (kCipItemIdSocketAddressInfoTargetToOriginator
        == common_packet_format_data->address_info_item[0].type_id) {
        port = common_packet_format_data->address_info_item[0].sin_port;
    } else {
        if (kCipItemIdSocketAddressInfoTargetToOriginator
            == common_packet_format_data->address_info_item[1].type_id) {
            port = common_packet_format_data->address_info_item[1].sin_port;
        }
    }

    connection_object->remote_address.sin_family = AF_INET;
    connection_object->remote_address.sin_addr.s_addr = 0; /* we don't know the address of the originate will be set in the IApp_CreateUDPSocket */
    connection_object->remote_address.sin_port = port;

    socket = CreateUdpSocket(kUdpCommuncationDirectionProducing,
        &connection_object->remote_address); /* the address is only needed for bind used if consuming */
    if (socket == kEipInvalidSocket) {
        OPENER_TRACE_ERR(
            "cannot create UDP socket in OpenPointToPointConnection\n");
        /* *pa_pnExtendedError = 0x0315; miscellaneous*/
        return kCipErrorConnectionFailure;
    }
    connection_object->socket[kUdpCommuncationDirectionProducing] = socket;

    return kCipStatusOk;
}

CipStatus OpenProducingMulticastConnection(
    CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data)
{
    CIP_Connection* existing_connection_object = GetExistingProducerMulticastConnection(
        connection_object->connection_path.connection_point[1]);
    int j;

    if (NULL == existing_connection_object) { /* we are the first connection producing for the given Input Assembly */
        return OpenMulticastConnection(kUdpCommuncationDirectionProducing,
            connection_object, common_packet_format_data);
    } else {
        /* we need to inform our originator on the correct connection id */
        connection_object->produced_connection_id = existing_connection_object
                                                        ->produced_connection_id;
    }

    /* we have a connection reuse the data and the socket */

    j = 0; /* allocate an unused sockaddr struct to use */
    if (g_common_packet_format_data_item.address_info_item[0].type_id == 0) { /* it is not used yet */
        j = 0;
    } else if (g_common_packet_format_data_item.address_info_item[1].type_id
        == 0) {
        j = 1;
    }

    if (kConnectionTypeIoExclusiveOwner == connection_object->instance_type) {
        /* exclusive owners take the socket and further manage the connection
     * especially in the case of time outs.
     */
        connection_object->socket[kUdpCommuncationDirectionProducing] = existing_connection_object->socket[kUdpCommuncationDirectionProducing];
        existing_connection_object->socket[kUdpCommuncationDirectionProducing] = kEipInvalidSocket;
    } else { /* this connection will not produce the data */
        connection_object->socket[kUdpCommuncationDirectionProducing] = kEipInvalidSocket;
    }

    common_packet_format_data->address_info_item[j].length = 16;
    common_packet_format_data->address_info_item[j].type_id = kCipItemIdSocketAddressInfoTargetToOriginator;
    connection_object->remote_address.sin_family = AF_INET;
    connection_object->remote_address.sin_port = common_packet_format_data
                                                     ->address_info_item[j]
                                                     .sin_port
        = htons(kOpenerEipIoUdpPort);
    connection_object->remote_address.sin_addr.s_addr = common_packet_format_data
                                                            ->address_info_item[j]
                                                            .sin_addr
        = g_multicast_configuration
              .starting_multicast_address;
    memset(common_packet_format_data->address_info_item[j].nasin_zero, 0, 8);
    common_packet_format_data->address_info_item[j].sin_family = htons(AF_INET);

    return kCipStatusOk;
}

/**  @brief Open a Multicast connection dependent on @var direction.
 *
 *   @param direction Flag to indicate if consuming or producing.
 *   @param connection_object  pointer to registered Object in ConnectionManager.
 *   @param common_packet_format_data     received CPF Data Item.
 *   @return status
 *               0 .. success
 *              -1 .. error
 */
CipStatus OpenMulticastConnection(
    UdpCommuncationDirection direction, CIP_Connection* connection_object,
    CipCommonPacketFormatData* common_packet_format_data)
{
    int j = 0;
    int socket;

    if (0 != g_common_packet_format_data_item.address_info_item[0].type_id) {
        if ((kUdpCommuncationDirectionConsuming == direction)
            && (kCipItemIdSocketAddressInfoOriginatorToTarget
                   == common_packet_format_data->address_info_item[0].type_id)) {
            /* for consuming connection points the originator can choose the multicast address to use
       * we have a given address type so use it */
        } else {
            j = 1;
            /* if the type is not zero (not used) or if a given type it has to be the correct one */
            if ((0 != g_common_packet_format_data_item.address_info_item[1].type_id)
                && (!((kUdpCommuncationDirectionConsuming == direction)
                       && (kCipItemIdSocketAddressInfoOriginatorToTarget
                              == common_packet_format_data->address_info_item[0].type_id)))) {
                OPENER_TRACE_ERR("no suitable addr info item available\n");
                return kCipStatusError;
            }
        }
    }

    if (0 == common_packet_format_data->address_info_item[j].type_id) { /* we are using an unused item initialize it with the default multicast address */
        common_packet_format_data->address_info_item[j].sin_family = htons(
            AF_INET);
        common_packet_format_data->address_info_item[j].sin_port = htons(
            kOpenerEipIoUdpPort);
        common_packet_format_data->address_info_item[j].sin_addr = g_multicast_configuration.starting_multicast_address;
        memset(common_packet_format_data->address_info_item[j].nasin_zero, 0, 8);
        common_packet_format_data->address_info_item[j].length = 16;
    }

    if (htons(AF_INET)
        != common_packet_format_data->address_info_item[j].sin_family) {
        OPENER_TRACE_ERR(
            "Sockaddr Info Item with wrong sin family value recieved\n");
        return kCipStatusError;
    }

    /* allocate an unused sockaddr struct to use */
    struct sockaddr_in socket_address;
    socket_address.sin_family = ntohs(
        common_packet_format_data->address_info_item[j].sin_family);
    socket_address.sin_addr.s_addr = common_packet_format_data->address_info_item[j].sin_addr;
    socket_address.sin_port = common_packet_format_data->address_info_item[j]
                                  .sin_port;

    socket = CreateUdpSocket(direction, &socket_address); /* the address is only needed for bind used if consuming */
    if (socket == kEipInvalidSocket) {
        OPENER_TRACE_ERR("cannot create UDP socket in OpenMulticastConnection\n");
        return kCipStatusError;
    }
    connection_object->socket[direction] = socket;

    if (direction == kUdpCommuncationDirectionConsuming) {
        common_packet_format_data->address_info_item[j].type_id = kCipItemIdSocketAddressInfoOriginatorToTarget;
        connection_object->originator_address = socket_address;
    } else {
        common_packet_format_data->address_info_item[j].type_id = kCipItemIdSocketAddressInfoTargetToOriginator;
        connection_object->remote_address = socket_address;
    }

    return kCipStatusOk;
}

CipUint HandleConfigData(CIP_ClassInstance* assembly_class,
    CIP_Connection* connection_object)
{
    CipUint connection_manager_status = 0;
    CIP_ClassInstance* config_instance = GetCIPClass(
        assembly_class, connection_object->connection_path.connection_point[2]);

    if (0 != g_config_data_length) {
        if (ConnectionWithSameConfigPointExists(
                connection_object->connection_path.connection_point[2])) { /* there is a connected connection with the same config point
         * we have to have the same data as already present in the config point*/
            CipByteArray* p = (CipByteArray*)GetCipAttribute(config_instance, 3)
                                  ->data;
            if (p->length != g_config_data_length) {
                connection_manager_status = kConnectionManagerStatusCodeErrorOwnershipConflict;
            } else {
                /*FIXME check if this is correct */
                if (memcmp(p->data, g_config_data_buffer, g_config_data_length)) {
                    connection_manager_status = kConnectionManagerStatusCodeErrorOwnershipConflict;
                }
            }
        } else {
            /* put the data on the configuration assembly object with the current
       design this can be done rather efficiently */
            if (kCipStatusOk
                != NotifyAssemblyConnectedDataReceived(config_instance,
                       g_config_data_buffer,
                       g_config_data_length)) {
                OPENER_TRACE_WARN("Configuration data was invalid\n");
                connection_manager_status = kConnectionManagerStatusCodeInvalidConfigurationApplicationPath;
            }
        }
    }
    return connection_manager_status;
}

void CloseIoConnection(CIP_Connection* connection_object)
{

    CheckIoConnectionEvent(connection_object->connection_path.connection_point[0],
        connection_object->connection_path.connection_point[1],
        kIoConnectionEventClosed);

    if ((kConnectionTypeIoExclusiveOwner == connection_object->instance_type)
        || (kConnectionTypeIoInputOnly == connection_object->instance_type)) {
        if ((kRoutingTypeMulticastConnection
                == (connection_object->t_to_o_network_connection_parameter
                       & kRoutingTypeMulticastConnection))
            && (kEipInvalidSocket
                   != connection_object->socket[kUdpCommuncationDirectionProducing])) {
            CIP_Connection* next_non_control_master_connection = GetNextNonControlMasterConnection(
                connection_object->connection_path.connection_point[1]);
            if (NULL != next_non_control_master_connection) {
                next_non_control_master_connection->socket[kUdpCommuncationDirectionProducing] = connection_object->socket[kUdpCommuncationDirectionProducing];
                memcpy(&(next_non_control_master_connection->remote_address),
                    &(connection_object->remote_address),
                    sizeof(next_non_control_master_connection->remote_address));
                next_non_control_master_connection->eip_level_sequence_count_producing = connection_object->eip_level_sequence_count_producing;
                next_non_control_master_connection->sequence_count_producing = connection_object->sequence_count_producing;
                connection_object->socket[kUdpCommuncationDirectionProducing] = kEipInvalidSocket;
                next_non_control_master_connection->transmission_trigger_timer = connection_object->transmission_trigger_timer;
            } else { /* this was the last master connection close all listen only connections listening on the port */
                CloseAllConnectionsForInputWithSameType(
                    connection_object->connection_path.connection_point[1],
                    kConnectionTypeIoListenOnly);
            }
        }
    }

    CloseCommunicationChannelsAndRemoveFromActiveConnectionsList(
        connection_object);
}

void HandleIoConnectionTimeOut(CIP_Connection* connection_object)
{
    CIP_Connection* next_non_control_master_connection;
    CheckIoConnectionEvent(connection_object->connection_path.connection_point[0],
        connection_object->connection_path.connection_point[1],
        kIoConnectionEventTimedOut);

    if (kRoutingTypeMulticastConnection
        == (connection_object->t_to_o_network_connection_parameter
               & kRoutingTypeMulticastConnection)) {
        switch (connection_object->instance_type) {
        case kConnectionTypeIoExclusiveOwner:
            CloseAllConnectionsForInputWithSameType(
                connection_object->connection_path.connection_point[1],
                kConnectionTypeIoInputOnly);
            CloseAllConnectionsForInputWithSameType(
                connection_object->connection_path.connection_point[1],
                kConnectionTypeIoListenOnly);
            break;
        case kConnectionTypeIoInputOnly:
            if (kEipInvalidSocket
                != connection_object->socket[kUdpCommuncationDirectionProducing]) { /* we are the controlling input only connection find a new controller*/
                next_non_control_master_connection = GetNextNonControlMasterConnection(
                    connection_object->connection_path.connection_point[1]);
                if (NULL != next_non_control_master_connection) {
                    next_non_control_master_connection->socket[kUdpCommuncationDirectionProducing] = connection_object->socket[kUdpCommuncationDirectionProducing];
                    connection_object->socket[kUdpCommuncationDirectionProducing] = kEipInvalidSocket;
                    next_non_control_master_connection->transmission_trigger_timer = connection_object->transmission_trigger_timer;
                } else { /* this was the last master connection close all listen only connections listening on the port */
                    CloseAllConnectionsForInputWithSameType(
                        connection_object->connection_path.connection_point[1],
                        kConnectionTypeIoListenOnly);
                }
            }
            break;
        default:
            break;
        }
    }

    OPENER_ASSERT(NULL != connection_object->connection_close_function);
    connection_object->connection_close_function(connection_object);
}

CipStatus SendConnectedData(CIP_Connection* connection_object)
{
    CipCommonPacketFormatData* common_packet_format_data;
    CipUint reply_length;
    CipUsint* message_data_reply_buffer;

    /* TODO think of adding an own send buffer to each connection object in order to preset up the whole message on connection opening and just change the variable data items e.g., sequence number */

    common_packet_format_data = &g_common_packet_format_data_item; /* TODO think on adding a CPF data item to the S_CIP_CIP_Connection in order to remove the code here or even better allocate memory in the connection object for storing the message to send and just change the application data*/

    connection_object->eip_level_sequence_count_producing++;

    /* assembleCPFData */
    common_packet_format_data->item_count = 2;
    if ((connection_object->transport_type_class_trigger & 0x0F) != 0) { /* use Sequenced Address Items if not Connection Class 0 */
        common_packet_format_data->address_item.type_id = kCipItemIdSequencedAddressItem;
        common_packet_format_data->address_item.length = 8;
        common_packet_format_data->address_item.data.sequence_number = connection_object->eip_level_sequence_count_producing;
    } else {
        common_packet_format_data->address_item.type_id = kCipItemIdConnectionAddress;
        common_packet_format_data->address_item.length = 4;
    }
    common_packet_format_data->address_item.data.connection_identifier = connection_object->produced_connection_id;

    common_packet_format_data->data_item.type_id = kCipItemIdConnectedDataItem;

    CipByteArray* producing_instance_attributes = (CipByteArray*)connection_object->producing_instance->attributes->data;
    common_packet_format_data->data_item.length = 0;

    /* notify the application that data will be sent immediately after the call */
    if (BeforeAssemblyDataSend(connection_object->producing_instance)) {
        /* the data has changed increase sequence counter */
        connection_object->sequence_count_producing++;
    }

    /* set AddressInfo Items to invalid Type */
    common_packet_format_data->address_info_item[0].type_id = 0;
    common_packet_format_data->address_info_item[1].type_id = 0;

    reply_length = AssembleIOMessage(common_packet_format_data,
        &g_message_data_reply_buffer[0]);

    message_data_reply_buffer = &g_message_data_reply_buffer[reply_length - 2];
    common_packet_format_data->data_item.length = producing_instance_attributes
                                                      ->length;
    if (kOpenerProducedDataHasRunIdleHeader) {
        common_packet_format_data->data_item.length += 4;
    }

    if ((connection_object->transport_type_class_trigger & 0x0F) == 1) {
        common_packet_format_data->data_item.length += 2;
        AddIntToMessage(common_packet_format_data->data_item.length,
            &message_data_reply_buffer);
        AddIntToMessage(connection_object->sequence_count_producing,
            &message_data_reply_buffer);
    } else {
        AddIntToMessage(common_packet_format_data->data_item.length,
            &message_data_reply_buffer);
    }

    if (kOpenerProducedDataHasRunIdleHeader) {
        AddDintToMessage(g_run_idle_state, &(message_data_reply_buffer));
    }

    memcpy(message_data_reply_buffer, producing_instance_attributes->data,
        producing_instance_attributes->length);

    reply_length += common_packet_format_data->data_item.length;

    return SendUdpData(
        &connection_object->remote_address,
        connection_object->socket[kUdpCommuncationDirectionProducing],
        &g_message_data_reply_buffer[0], reply_length);
}

CipStatus HandleReceivedIoConnectionData(CIP_Connection* connection_object,
    CipUsint* data, CipUint data_length)
{

    /* check class 1 sequence number*/
    if ((connection_object->transport_type_class_trigger & 0x0F) == 1) {
        CipUint sequence_buffer = GetIntFromMessage(&(data));
        if (SEQ_LEQ16(sequence_buffer,
                connection_object->sequence_count_consuming)) {
            return kCipStatusOk; /* no new data for the assembly */
        }
        connection_object->sequence_count_consuming = sequence_buffer;
        data_length -= 2;
    }

    if (data_length > 0) {
        /* we have no heartbeat connection */
        if (kOpenerConsumedDataHasRunIdleHeader) {
            CipUdint nRunIdleBuf = GetDintFromMessage(&(data));
            if (g_run_idle_state != nRunIdleBuf) {
                RunIdleChanged(nRunIdleBuf);
            }
            g_run_idle_state = nRunIdleBuf;
            data_length -= 4;
        }

        if (NotifyAssemblyConnectedDataReceived(
                connection_object->consuming_instance, data, data_length)
            != 0) {
            return kCipStatusError;
        }
    }
    return kCipStatusOk;
}

CipStatus OpenCommunicationChannels(CIP_Connection* connection_object)
{

    CipStatus eip_status = kCipStatusOk;
    /*get pointer to the CPF data, currently we have just one global instance of the struct. This may change in the future*/
    CipCommonPacketFormatData* common_packet_format_data = &g_common_packet_format_data_item;

    int originator_to_target_connection_type = (connection_object
                                                       ->o_to_t_network_connection_parameter
                                                   & 0x6000)
        >> 13;

    int target_to_originator_connection_type = (connection_object
                                                       ->t_to_o_network_connection_parameter
                                                   & 0x6000)
        >> 13;

    /* open a connection "point to point" or "multicast" based on the ConnectionParameter */
    if (originator_to_target_connection_type == 1) /*TODO: Fix magic number; Multicast consuming */
    {
        if (OpenMulticastConnection(kUdpCommuncationDirectionConsuming,
                connection_object, common_packet_format_data)
            == kCipStatusError) {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return kCipErrorConnectionFailure;
        }
    } else if (originator_to_target_connection_type == 2) /* TODO: Fix magic number; Point to Point consuming */
    {
        if (OpenConsumingPointToPointConnection(connection_object,
                common_packet_format_data)
            == kCipStatusError) {
            OPENER_TRACE_ERR("error in PointToPoint consuming connection\n");
            return kCipErrorConnectionFailure;
        }
    }

    if (target_to_originator_connection_type == 1) /* TODO: Fix magic number; Multicast producing */
    {
        if (OpenProducingMulticastConnection(connection_object,
                common_packet_format_data)
            == kCipStatusError) {
            OPENER_TRACE_ERR("error in OpenMulticast Connection\n");
            return kCipErrorConnectionFailure;
        }
    } else if (target_to_originator_connection_type == 2) /* TODO: Fix magic number; Point to Point producing */
    {

        if (OpenProducingPointToPointConnection(connection_object,
                common_packet_format_data)
            != kCipStatusOk) {
            OPENER_TRACE_ERR("error in PointToPoint producing connection\n");
            return kCipErrorConnectionFailure;
        }
    }

    return eip_status;
}

void CloseCommunicationChannelsAndRemoveFromActiveConnectionsList(
    CIP_Connection* connection_object)
{
    IApp_CloseSocket_udp(
        connection_object->socket[kUdpCommuncationDirectionConsuming]);
    connection_object->socket[kUdpCommuncationDirectionConsuming] = kEipInvalidSocket;
    IApp_CloseSocket_udp(
        connection_object->socket[kUdpCommuncationDirectionProducing]);
    connection_object->socket[kUdpCommuncationDirectionProducing] = kEipInvalidSocket;

    RemoveFromActiveConnections(connection_object);
}
