/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/

#include "CIP_Connection.h"
#include <cstring>
#include "CIP_Class3Connection.h"
#include "../CIP_Identity.h"
#include "CIP_IOConnection.h"
#include "../../utils/UTIL_Endianconv.h"
#include "../../trace.h"

#ifdef WIN32
#include "winsock.h"
#else
#include "arpa/inet"
#endif

#define CIP_CONN_TYPE_MASK 0x6000 /**< Bit mask filter on bit 13 & 14 */

const int g_kForwardOpenHeaderLength = 36; /**< the length in bytes of the forward open command specific data till the start of the connection path (including con path size)*/

/** @brief Compares the logical path on equality */
#define EQLOGICALPATH(x, y) (((x)&0xfc) == (y))

static const int g_kNumberOfConnectableObjects = 2 + OPENER_CIP_NUM_APPLICATION_SPECIFIC_CONNECTABLE_OBJECTS;

CIP_Connection::CIP_Connection (struct sockaddr *originator_address, struct sockaddr *remote_address)
{
    netConn = new NET_Connection(originator_address, remote_address);
}

CIP_Connection::~CIP_Connection ()
{
    delete (netConn);
}

/** @brief gets the padded logical path TODO: enhance documentation
 * @param logical_path_segment TheLogical Path Segment
 *
 */
unsigned int CIP_Connection::GetPaddedLogicalPath (unsigned char **logical_path_segment)
{
    unsigned int padded_logical_path = *(*logical_path_segment)++;

    if ((padded_logical_path & 3) == 0)
    {
        padded_logical_path = *(*logical_path_segment)++;
    }
    else if ((padded_logical_path & 3) == 1)
    {
        (*logical_path_segment)++; // skip pad
        padded_logical_path = *(*logical_path_segment)++;
        padded_logical_path |= *(*logical_path_segment)++ << 8;
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
CipUdint CIP_Connection::GetConnectionId (void)
{
    static CipUdint connection_id = 18;
    connection_id++;
    return (g_incarnation_id | (connection_id & 0x0000FFFF));
}


CipStatus CIP_Connection::ConnectionManagerInit (CipUint unique_connection_id)
{
    CIP_Class3conn::InitializeClass3ConnectionData ();
    CIP_IOConnection::InitializeIOConnectionData ();

    class_id = kCipConnectionManagerClassCode;
    get_all_class_attributes_mask = 0xC6;
    get_all_instance_attributes_mask = 0xffffffff;
    class_name = "Connection Manager";
    revision = 1;

    g_incarnation_id = ((CipUdint) unique_connection_id) << 16;


    return kCipStatusOk;
}

CipStatus CIP_Connection::HandleReceivedConnectedData (CipUsint *data, int data_length, struct sockaddr_in *from_address)
{

    if ((CIP_CommonPacket::CreateCommonPacketFormatStructure (data, data_length, &CIP_CommonPacket::common_packet_data)) == kCipStatusError)
    {
        return kCipStatusError;
    }
    else
    {
        // check if connected address item or sequenced address item  received, otherwise it is no connected message and should not be here
        if ((CIP_CommonPacket::common_packet_data.address_item.type_id == CIP_CommonPacket::kCipItemIdConnectionAddress) || (CIP_CommonPacket::common_packet_data.address_item.type_id == CIP_CommonPacket::kCipItemIdSequencedAddressItem))
        {
            // found connected address item or found sequenced address item -> for now the sequence number will be ignored
            if (CIP_CommonPacket::common_packet_data.data_item.type_id == CIP_CommonPacket::kCipItemIdConnectedDataItem)
            {
                // connected data item received
                //CIP_Connection *connection_object = GetConnectedObject (CIP_CommonPacket::common_packet_data.address_item.data.connection_identifier);
                //if (connection_object == NULL)
                //    return kCipStatusError;

                // only handle the data if it is coming from the originator
                if (((struct sockaddr_in*)(netConn->originator_address))->sin_addr.s_addr == from_address->sin_addr.s_addr)
                {
                    if (SEQ_GT32(CIP_CommonPacket::common_packet_data.address_item.data.sequence_number, eip_level_sequence_count_consuming))
                    {
                        // reset the watchdog timer
                        inactivity_watchdog_timer = (o_to_t_requested_packet_interval / 1000) << (2 + connection_timeout_multiplier);

                        // only inform assembly object if the sequence counter is greater or equal
                        eip_level_sequence_count_consuming = CIP_CommonPacket::common_packet_data.address_item.data.sequence_number;

                        //TODO: fix handles per IO Type
                        //return HandleReceivedIoConnectionData (CIP_CommonPacket::common_packet_data.data_item.data, CIP_CommonPacket::common_packet_data.data_item.length);
                    }
                } else
                {
                    OPENER_TRACE_WARN("Connected Message Data Received with wrong address information\n");
                }
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
CipStatus CIP_Connection::ForwardOpen (CipMessageRouterRequest *message_router_request, CipMessageRouterResponse *message_router_response)
{
    CipUint connection_status = kConnectionManagerStatusCodeSuccess;
    ConnectionManagementHandling *connection_management_entry;

    //first check if we have already a connection with the given params
    priority_timetick = *message_router_request->data++;
    timeout_ticks = *message_router_request->data++;
    // O_to_T netConn ID
    consumed_connection_id = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);
    // T_to_O netConn ID
    produced_connection_id = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);
    connection_serial_number = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);
    originator_vendor_id = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);
    originator_serial_number = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);

    if ((NULL != CheckForExistingConnection (&g_dummy_connection_object)))
    {
        // TODO this test is  incorrect, see CIP spec 3-5.5.2 re: duplicate forward open
        // it should probably be testing the connection type fields
        // TODO think on how a reconfiguration request could be handled correctly
        if ((0 == consumed_connection_id) && (0 == produced_connection_id))
        {
            //TODO implement reconfiguration of connection

            OPENER_TRACE_ERR("this looks like a duplicate forward open -- I can't handle this yet, sending a CIP_CON_MGR_ERROR_CONNECTION_IN_USE response\n");
        }
        return AssembleForwardOpenResponse (message_router_response, kCipErrorConnectionFailure, kConnectionManagerStatusCodeErrorConnectionInUse);
    }
    // keep it to none existent till the setup is done this eases error handling and
    // the state changes within the forward open request can not be detected from
    // the application or from outside (reason we are single threaded)
    state = kConnectionStateNonExistent;
    sequence_count_producing = 0; // set the sequence count to zero

    connection_timeout_multiplier = *message_router_request->data++;
    message_router_request->data += 3; // reserved
    // the requested packet interval parameter needs to be a multiple of TIMERTICK from the header file
    OPENER_TRACE_INFO("ForwardOpen: ConConnID %"PRIu32", ProdConnID %"PRIu32", ConnSerNo %u\n", consumed_connection_id, produced_connection_id, connection_serial_number);

    o_to_t_requested_packet_interval = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);

    o_to_t_network_connection_parameter = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);
    t_to_o_requested_packet_interval = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);

    CipUdint temp = t_to_o_requested_packet_interval % (kOpenerTimerTickInMilliSeconds * 1000);
    if (temp > 0)
    {
        t_to_o_requested_packet_interval = (CipUdint) (t_to_o_requested_packet_interval / (kOpenerTimerTickInMilliSeconds * 1000)) * (kOpenerTimerTickInMilliSeconds * 1000) + (kOpenerTimerTickInMilliSeconds * 1000);
    }

    t_to_o_network_connection_parameter = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);

    //check if Network connection parameters are ok
    if (CIP_CONN_TYPE_MASK == (o_to_t_network_connection_parameter & CIP_CONN_TYPE_MASK))
    {
        return AssembleForwardOpenResponse ( message_router_response, kCipErrorConnectionFailure, kConnectionManagerStatusCodeErrorInvalidOToTConnectionType);
    }

    if (CIP_CONN_TYPE_MASK == (t_to_o_network_connection_parameter & CIP_CONN_TYPE_MASK))
    {
        return AssembleForwardOpenResponse ( message_router_response, kCipErrorConnectionFailure, kConnectionManagerStatusCodeErrorInvalidTToOConnectionType);
    }

    transport_type_class_trigger = *message_router_request->data++;
    //check if the trigger type value is ok
    if (0x40 & transport_type_class_trigger)
    {
        return AssembleForwardOpenResponse ( message_router_response, kCipErrorConnectionFailure, kConnectionManagerStatusCodeErrorTransportTriggerNotSupported);
    }

    temp = ParseConnectionPath ( message_router_request, &connection_status);
    if (kCipStatusOk != temp)
    {
        return AssembleForwardOpenResponse ( message_router_response, (CipUsint)temp, connection_status);
    }

    //parsing is now finished all data is available and check now establish the connection
    connection_management_entry = GetConnMgmEntry (connection_path.class_id);
    if (NULL != connection_management_entry)
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
        return this->AssembleForwardOpenResponse (message_router_response, (CipUsint)temp, connection_status);
    }
    else
    {
        OPENER_TRACE_INFO("connection manager: connect succeeded\n");
        // in case of success the g_pstActiveConnectionList points to the new connection
        return this->AssembleForwardOpenResponse (message_router_response, kCipErrorSuccess, 0);
    }
}

void CIP_Connection::GeneralConnectionConfiguration ()
{
    if (kRoutingTypePointToPointConnection == (this->o_to_t_network_connection_parameter & kRoutingTypePointToPointConnection))
    {
        // if we have a point to point connection for the O to T direction the target shall choose the connection ID.
        consumed_connection_id = GetConnectionId ();
    }

    if (kRoutingTypeMulticastConnection == (this->t_to_o_network_connection_parameter & kRoutingTypeMulticastConnection))
    {
        // if we have a multi-cast connection for the T to O direction the target shall choose the connection ID.
        produced_connection_id = GetConnectionId ();
    }

    eip_level_sequence_count_producing = 0;
    sequence_count_producing = 0;
    eip_level_sequence_count_consuming = 0;
    sequence_count_consuming = 0;

    watchdog_timeout_action = kWatchdogTimeoutActionAutoDelete; /* the default for all connections on EIP*/

    expected_packet_rate = 0; // default value

    // Client Type Connection requested
    if ((transport_type_class_trigger & 0x80) == 0x00)
    {
        expected_packet_rate = (CipUint) ((t_to_o_requested_packet_interval) / 1000);
        // As soon as we are ready we should produce the connection. With the 0 here we will produce with the next timer tick which should be sufficient.
        transmission_trigger_timer = 0;
    }
    else
    {
        // Server Type Connection requested
        expected_packet_rate = (CipUint) ((o_to_t_requested_packet_interval) / 1000);
    }

    this->production_inhibit_timer = production_inhibit_time = 0;

    //setup the preconsuption timer: max(ConnectionTimeoutMultiplier * EpectetedPacketRate, 10s)
    this->inactivity_watchdog_timer = ((((o_to_t_requested_packet_interval) / 1000) << (2 + connection_timeout_multiplier)) > 10000) ? (((this->o_to_t_requested_packet_interval) / 1000) << (2 + this->connection_timeout_multiplier)) : 10000;

    this->consumed_connection_size = (CipUint)(o_to_t_network_connection_parameter & 0x01FF);

    this->produced_connection_size = (CipUint)(t_to_o_network_connection_parameter & 0x01FF);
}

CipStatus CIP_Connection::ForwardClose (CipMessageRouterRequest *message_router_request, CipMessageRouterResponse *message_router_response)
{

    // check connection_serial_number && originator_vendor_id && originator_serial_number if connection is established
    ConnectionManagerStatusCode connection_status = kConnectionManagerStatusCodeErrorConnectionNotFoundAtTargetApplication;
    CIP_Connection *connection_object;

    // set AddressInfo Items to invalid TypeID to prevent assembleLinearMsg to read them
    CIP_CommonPacket::common_packet_data.address_info_item[0].type_id = 0;
    CIP_CommonPacket::common_packet_data.address_info_item[1].type_id = 0;

    message_router_request->data += 2; // ignore Priority/Time_tick and Time-out_ticks

    CipUint connection_serial_number = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);
    CipUint originator_vendor_id = UTIL_Endianconv::GetIntFromMessage (&message_router_request->data);
    CipUdint originator_serial_number = UTIL_Endianconv::GetDintFromMessage (&message_router_request->data);

    OPENER_TRACE_INFO("ForwardClose: ConnSerNo %d\n", connection_serial_number);

    for (int i = 0; i < active_connections_set.size (); i++)
    {
        connection_object = active_connections_set[i];
        /* this check should not be necessary as only established connections should be in the active connection list */
        if ((connection_object->state == kConnectionStateEstablished) || (connection_object->state == kConnectionStateTimedOut))
        {
            if ((connection_object->connection_serial_number == connection_serial_number) &&
                (connection_object->originator_vendor_id == originator_vendor_id) &&
                (connection_object->originator_serial_number == originator_serial_number))
            {
                /* found the corresponding connection object -> close it */
                //OPENER_ASSERT(NULL != connection_object->connection_close_function);
                CloseConnection ();
                connection_status = kConnectionManagerStatusCodeSuccess;
                break;
            }
        }
    }

    return AssembleForwardCloseResponse (connection_serial_number, originator_vendor_id, originator_serial_number, message_router_request, message_router_response, connection_status);
}

/* TODO: Not implemented */
CipStatus CIP_Connection::GetConnectionOwner (CipMessageRouterRequest *message_router_request, CipMessageRouterResponse *message_router_response)
{
    /* suppress compiler warnings */
    (void) message_router_request;
    (void) message_router_response;

    return kCipStatusOk;
}

CipStatus CIP_Connection::ManageConnections (MilliSeconds elapsed_time)
{
    CipStatus eip_status;
    CIP_Connection *connection_object;

    /*Inform application that it can execute */
    //todo:HandleApplication ();
    ManageEncapsulationMessages (elapsed_time);

    for (int i = 0; i < active_connections_set.size (); i++)
    {
        connection_object = active_connections_set[i];

        if (connection_object->state == kConnectionStateEstablished)
        {
            if ((0 != connection_object->consuming_instance) ||
                // we have a consuming connection check inactivity watchdog timer
                (connection_object->transport_type_class_trigger &
                 0x80)) // all sever connections have to maintain an inactivity watchdog timer
            {
                connection_object->inactivity_watchdog_timer -= elapsed_time;
                if (connection_object->inactivity_watchdog_timer <= 0)
                {
                    // we have a timed out connection perform watchdog time out action
                    OPENER_TRACE_INFO(">>>>>>>>>>Connection timed out\n");
                    //OPENER_ASSERT(NULL != connection_object->connection_timeout_function);
                    connection_object->RemoveFromActiveConnections ();
                }
            }
            // only if the connection has not timed out check if data is to be send
            if (kConnectionStateEstablished == connection_object->state)
            {
                // client connection
                if ((connection_object->expected_packet_rate != 0) && (kEipInvalidSocket != connection_object->netConn->sock)) // only produce for the master connection
                {
                    if (kConnectionTriggerTypeCyclicConnection != (connection_object->transport_type_class_trigger & kConnectionTriggerTypeProductionTriggerMask))
                    {
                        // non cyclic connections have to decrement production inhibit timer
                        if (0 <= connection_object->production_inhibit_timer)
                        {
                            connection_object->production_inhibit_timer -= elapsed_time;
                        }
                    }
                    connection_object->transmission_trigger_timer -= elapsed_time;
                    if (connection_object->transmission_trigger_timer <= 0)
                    {
                        // need to send package
                        //OPENER_ASSERT(NULL != connection_object->connection_send_data_function);
                        //todo: eip_status = SendConnectedData (connection_object); //only for IO connections
                        if (eip_status == kCipStatusError)
                        {
                            OPENER_TRACE_ERR("sending of UDP data in manage Connection failed\n");
                        }
                        // reload the timer value
                        connection_object->transmission_trigger_timer = connection_object->expected_packet_rate;
                        if (kConnectionTriggerTypeCyclicConnection != (connection_object->transport_type_class_trigger & kConnectionTriggerTypeProductionTriggerMask))
                        {
                            // non cyclic connections have to reload the production inhibit timer
                            connection_object->production_inhibit_timer = connection_object->production_inhibit_time;
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
CipStatus CIP_Connection::AssembleForwardOpenResponse (CipMessageRouterResponse *message_router_response, CipUsint general_status, CipUint extended_status)
{
    /* write reply information in CPF struct dependent of pa_status */
    CIP_CommonPacket::PacketFormat *cip_common_packet_format_data = &CIP_CommonPacket::common_packet_data;
    CipByte *message = message_router_response->data;
    cip_common_packet_format_data->item_count = 2;
    cip_common_packet_format_data->data_item.type_id = CIP_CommonPacket::kCipItemIdUnconnectedDataItem;

    AddNullAddressItem (cip_common_packet_format_data);

    message_router_response->reply_service = (0x80 | kForwardOpen);
    message_router_response->general_status = general_status;

    if (kCipErrorSuccess == general_status)
    {
        OPENER_TRACE_INFO("assembleFWDOpenResponse: sending success response\n");
        message_router_response->data_length = 26; /* if there is no application specific data */
        message_router_response->size_of_additional_status = 0;

        if (cip_common_packet_format_data->address_info_item[0].type_id != 0)
        {
            cip_common_packet_format_data->item_count = 3;
            if (cip_common_packet_format_data->address_info_item[1].type_id != 0)
            {
                cip_common_packet_format_data->item_count = 4; /* there are two sockaddrinfo items to add */
            }
        }

        UTIL_Endianconv::AddDintToMessage (consumed_connection_id, &message);
        UTIL_Endianconv::AddDintToMessage (produced_connection_id, &message);
    }
    else
    {
        /* we have an connection creation error */
        OPENER_TRACE_INFO("assembleFWDOpenResponse: sending error response\n");
        state = kConnectionStateNonExistent;
        message_router_response->data_length = 10;

        switch (general_status)
        {
            case kCipErrorNotEnoughData:
            case kCipErrorTooMuchData:
            {
                message_router_response->size_of_additional_status = 0;
                break;
            }

            default:
            {
                switch (extended_status)
                {
                    case kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize:
                    {
                        message_router_response->size_of_additional_status = 2;
                        message_router_response->additional_status[0] = extended_status;
                        message_router_response->additional_status[1] = correct_originator_to_target_size;
                        break;
                    }

                    case kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize:
                    {
                        message_router_response->size_of_additional_status = 2;
                        message_router_response->additional_status[0] = extended_status;
                        message_router_response->additional_status[1] = correct_target_to_originator_size;
                        break;
                    }

                    default:
                    {
                        message_router_response->size_of_additional_status = 1;
                        message_router_response->additional_status[0] = extended_status;
                        break;
                    }
                }
                break;
            }
        }
    }

    UTIL_Endianconv::AddIntToMessage (connection_serial_number, &message);
    UTIL_Endianconv::AddIntToMessage (originator_vendor_id, &message);
    UTIL_Endianconv::AddDintToMessage (originator_serial_number, &message);

    if (kCipErrorSuccess == general_status)
    {
        // set the actual packet rate to requested packet rate
        UTIL_Endianconv::AddDintToMessage (o_to_t_requested_packet_interval, &message);
        UTIL_Endianconv::AddDintToMessage (t_to_o_requested_packet_interval, &message);
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
void CIP_Connection::AddNullAddressItem (CIP_CommonPacket::PacketFormat *common_data_packet_format_data)
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
CipStatus CIP_Connection::AssembleForwardCloseResponse (CipUint connection_serial_number, CipUint originatior_vendor_id, CipUdint originator_serial_number, CipMessageRouterRequest *message_router_request, CipMessageRouterResponse *message_router_response, CipUint extended_error_code)
{
    // write reply information in CPF struct dependent of pa_status
    CIP_CommonPacket::PacketFormat *common_data_packet_format_data = &CIP_CommonPacket::common_packet_data;
    CipByte *message = message_router_response->data;
    common_data_packet_format_data->item_count = 2;
    common_data_packet_format_data->data_item.type_id = CIP_CommonPacket::kCipItemIdUnconnectedDataItem;

    AddNullAddressItem (common_data_packet_format_data);

    UTIL_Endianconv::AddIntToMessage (connection_serial_number, &message);
    UTIL_Endianconv::AddIntToMessage (originatior_vendor_id, &message);
    UTIL_Endianconv::AddDintToMessage (originator_serial_number, &message);

    message_router_response->reply_service = (CipUsint)(0x80 | message_router_request->service);
    message_router_response->data_length = 10; // if there is no application specific data

    if (kConnectionManagerStatusCodeSuccess == extended_error_code)
    {
        *message = 0; // no application data
        message_router_response->general_status = kCipErrorSuccess;
        message_router_response->size_of_additional_status = 0;
    }
    else
    {
        *message = *message_router_request->data; // remaining path size
        message_router_response->general_status = kCipErrorConnectionFailure;
        message_router_response->additional_status[0] = extended_error_code;
        message_router_response->size_of_additional_status = 1;
    }

    message++;
    *message = 0; // reserved
    message++;

    return kCipStatusOkSend;
}

CIP_Connection *CIP_Connection::GetConnectedObject (CipUdint connection_id)
{
    CIP_Connection *active_connection_object_list_item;;

    for (int i = 0; i < CIP_Connection::active_connections_set.size (); i++)
    {
        active_connection_object_list_item = active_connections_set[i];

        if (active_connection_object_list_item->state == kConnectionStateEstablished)
        {
            if (active_connection_object_list_item->consumed_connection_id == connection_id)
                return active_connection_object_list_item;
        }
    }
    return NULL;
}

CIP_Connection *CIP_Connection::GetConnectedOutputAssembly (CipUdint output_assembly_id)
{
    CIP_Connection *active_connection_object_list_item;

    for (int i = 0; i < CIP_Connection::active_connections_set.size (); i++)
    {
        active_connection_object_list_item = active_connections_set[i];

        if (active_connection_object_list_item->state == kConnectionStateEstablished)
        {
            if (active_connection_object_list_item->connection_path.connection_point[0] == output_assembly_id)
                return active_connection_object_list_item;
        }
    }
    return NULL;
}

CIP_Connection *CIP_Connection::CheckForExistingConnection (CIP_Connection *connection_object)
{
    CIP_Connection *active_connection_object_list_item;
    for (int i = 0; i < CIP_Connection::active_connections_set.size (); i++)
    {
        active_connection_object_list_item = active_connections_set[i];

        if (active_connection_object_list_item->state == kConnectionStateEstablished)
        {
            if ((connection_object->connection_serial_number == active_connection_object_list_item->connection_serial_number) &&
                (connection_object->originator_vendor_id == active_connection_object_list_item->originator_vendor_id) &&
                (connection_object->originator_serial_number == active_connection_object_list_item->originator_serial_number))
            {
                return active_connection_object_list_item;
            }
        }
    }
    return NULL;
}

CipStatus CIP_Connection::CheckElectronicKeyData (CipUsint key_format, CipKeyData *key_data, CipUint *extended_status)
{
    CipByte compatiblity_mode = (CipByte) (key_data->major_revision & 0x80);

    // Remove compatibility bit
    key_data->major_revision &= 0x7F;

    // Default return value
    *extended_status = kConnectionManagerStatusCodeSuccess;

    // Check key format
    if (4 != key_format)
    {
        *extended_status = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
        return kCipStatusError;
    }

    // Check VendorID and ProductCode, must match, or 0
    if (((key_data->vendor_id != CIP_Identity::vendor_id_) && (key_data->vendor_id != 0)) || ((key_data->product_code != CIP_Identity::product_code_) && (key_data->product_code != 0)))
    {
        *extended_status = kConnectionManagerStatusCodeErrorVendorIdOrProductcodeError;
        return kCipStatusError;
    }
    else
    {
        // VendorID and ProductCode are correct

        // Check DeviceType, must match or 0
        if ((key_data->device_type != CIP_Identity::device_type_) && (key_data->device_type != 0))
        {
            *extended_status = kConnectionManagerStatusCodeErrorDeviceTypeError;
            return kCipStatusError;
        }
        else
        {
            // VendorID, ProductCode and DeviceType are correct

            if (!compatiblity_mode)
            {
                // Major = 0 is valid
                if (0 == key_data->major_revision)
                {
                    return (kCipStatusOk);
                }

                // Check Major / Minor Revision, Major must match, Minor match or 0
                if ((key_data->major_revision != CIP_Identity::revision_.major_revision) || ((key_data->minor_revision != CIP_Identity::revision_.minor_revision) && (key_data->minor_revision != 0)))
                {
                    *extended_status = kConnectionManagerStatusCodeErrorRevisionMismatch;
                    return kCipStatusError;
                }
            } else
            {
                // Compatibility mode is set

                // Major must match, Minor != 0 and <= MinorRevision
                if ((key_data->major_revision == CIP_Identity::revision_.major_revision) && (key_data->minor_revision > 0) && (key_data->minor_revision <= CIP_Identity::revision_.minor_revision))
                {
                    return (kCipStatusOk);
                }
                else
                {
                    *extended_status = kConnectionManagerStatusCodeErrorRevisionMismatch;
                    return kCipStatusError;
                }
            } // end if CompatiblityMode handling
        }
    }

    return (*extended_status == kConnectionManagerStatusCodeSuccess) ? kCipStatusOk : kCipStatusError;
}

CipUsint CIP_Connection::ParseConnectionPath (CipMessageRouterRequest *message_router_request, CipUint *extended_error)
{
    CipUsint *message = message_router_request->data;
    int remaining_path_size = connection_path_size = *message++; // length in words

    int originator_to_target_connection_type;
    int target_to_originator_connection_type;

    // with 256 we mark that we haven't got a PIT segment
    production_inhibit_time = 256;

    if ((g_kForwardOpenHeaderLength + remaining_path_size * 2) < message_router_request->data_length)
    {
        // the received packet is larger than the data in the path
        *extended_error = 0;
        return kCipErrorTooMuchData;
    }

    if ((g_kForwardOpenHeaderLength + remaining_path_size * 2) > message_router_request->data_length)
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
            electronic_key.segment_type = 0x34;
            message++;
            electronic_key.key_format = *message++;
            electronic_key.key_data.vendor_id = UTIL_Endianconv::GetIntFromMessage (&message);
            electronic_key.key_data.device_type = UTIL_Endianconv::GetIntFromMessage (&message);
            electronic_key.key_data.product_code = UTIL_Endianconv::GetIntFromMessage (&message);
            electronic_key.key_data.major_revision = *message++;
            electronic_key.key_data.minor_revision = *message++;
            remaining_path_size -= 5; //length of the electronic key
            OPENER_TRACE_INFO("key: ven ID %d, dev type %d, prod code %d, major %d, minor %d\n",
                              connection_object->electronic_key.key_data.vendor_id,
                              connection_object->electronic_key.key_data.device_type,
                              connection_object->electronic_key.key_data.product_code,
                              connection_object->electronic_key.key_data.major_revision,
                              connection_object->electronic_key.key_data.minor_revision);

            if (kCipStatusOk != CheckElectronicKeyData (electronic_key.key_format, &(electronic_key.key_data), extended_error))
            {
                return kCipErrorConnectionFailure;
            }
        } else
        {
            OPENER_TRACE_INFO("no key\n");
        }

        if (kConnectionTriggerTypeCyclicConnection != (transport_type_class_trigger & kConnectionTriggerTypeProductionTriggerMask))
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
            connection_path.class_id = GetPaddedLogicalPath (&message);
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
            connection_path.connection_point[2] = GetPaddedLogicalPath (&message);
            OPENER_TRACE_INFO("Configuration instance id %"PRId32"\n", connection_path.connection_point[2]);
            //todo: fix this -> if (NULL == connection_path.connection_point[2])
            {
                // according to the test tool we should respond with this extended error code
                *extended_error = kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath;
                return kCipErrorConnectionFailure;
            }
            // 1 or 2 16Bit words for the configuration instance part of the path
            remaining_path_size -= (connection_path.connection_point[2] > 0xFF) ? 2 : 1;
        } else
        {
            OPENER_TRACE_INFO("no config data\n");
        }

        if (0x03 == (transport_type_class_trigger & 0x03))
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
                    connection_path.connection_point[i] = GetPaddedLogicalPath (&message);
                    OPENER_TRACE_INFO("connection point %"PRIu32"\n", connection_path.connection_point[i]);
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

            CIP_IOConnection::g_config_data_length = 0;
            CIP_IOConnection::g_config_data_buffer = NULL;

            while (remaining_path_size > 0)
            {
                // have something left in the path should be configuration data

                switch (*message)
                {
                    case kDataSegmentTypeSimpleDataMessage:
                        // we have a simple data segment
                        CIP_IOConnection::g_config_data_length = message[1] * 2; //data segments store length 16-bit word wise
                        CIP_IOConnection::g_config_data_buffer = &(message[2]);
                        remaining_path_size -= (CIP_IOConnection::g_config_data_length + 2);
                        message += (CIP_IOConnection::g_config_data_length + 2);
                        break;
                        // TODO do we have to handle ANSI extended symbol data segments too?
                    case kProductionTimeInhibitTimeNetworkSegment:
                        if (kConnectionTriggerTypeCyclicConnection != (transport_type_class_trigger & kConnectionTriggerTypeProductionTriggerMask))
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
                        *extended_error = connection_path_size - remaining_path_size;

                        //status code for invalid segment type
                        return 0x04;
                        break;
                }
            }
        }
    }

    // save back the current position in the stream allowing followers to parse anything thats still there
    message_router_request->data = message;
    return kCipStatusOk;
}

void CIP_Connection::CloseConnection ()
{
    state = kConnectionStateNonExistent;
    if (0x03 != (transport_type_class_trigger & 0x03))
    {
        // only close the UDP connection for not class 3 connections
        //IApp_CloseSocket_udp (pa_pstConnObj->netConn->GetSocketHandle());
        //netConn->SetSocketHandle(kEipInvalidSocket);
        netConn->CloseSocket ();
    }
    RemoveFromActiveConnections ();
}

void CIP_Connection::CopyConnectionData (CIP_Connection *pa_pstDst, CIP_Connection *pa_pstSrc)
{
    memcpy (pa_pstDst, pa_pstSrc, sizeof (CIP_Connection));
}

void CIP_Connection::AddNewActiveConnection (CIP_Connection *pa_pstConn)
{
    pa_pstConn->state = kConnectionStateEstablished;
}

void CIP_Connection::RemoveFromActiveConnections ()
{
    state = kConnectionStateNonExistent;
    active_connections_set.erase (id);
}

CipBool CIP_Connection::IsConnectedOutputAssembly (CipUdint pa_nInstanceNr)
{
    CipBool bRetVal = (CipBool)false;

    CIP_Connection *pstRunner;

    for (int i = 0; i < active_connections_set.size (); i++)
    {
        pstRunner = active_connections_set[i];
        if (pa_nInstanceNr == pstRunner->connection_path.connection_point[0])
        {
            bRetVal = (CipBool)true;
            break;
        }
    }
    return bRetVal;
}


CipStatus CIP_Connection::TriggerConnections (CipUdint pa_unOutputAssembly, CipUdint pa_unInputAssembly)
{
    CipStatus nRetVal = kCipStatusError;

    CIP_Connection *pstRunner;

    for (int i = 0; i < active_connections_set.size (); i++)
    {
        pstRunner = active_connections_set[i];

        if ((pa_unOutputAssembly == pstRunner->connection_path.connection_point[0]) &&
            (pa_unInputAssembly == pstRunner->connection_path.connection_point[1]))
        {
            if (kConnectionTriggerTypeApplicationTriggeredConnection == (pstRunner->transport_type_class_trigger & kConnectionTriggerTypeProductionTriggerMask))
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

CipStatus CIP_Connection::InstanceServices(int service, CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{
    //Class services
    if (this->id == 0)
    {
        switch(service)
        {
            case kForwardOpen:
                break;
            case kForwardClose:
                break;
            case kGetConnectionOwner:
                break;
        }
    }
    //Instance services
    else
    {

    }
}