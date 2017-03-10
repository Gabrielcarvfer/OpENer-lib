/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <cip/CIP_Objects/CIP_0005_Connection/CIP_Connection.hpp>
#include <cip/ciptypes.hpp>
#include "CIP_CommonPacket.hpp"
#include "../CIP_Common.hpp"
#include "cip/CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "../CIP_Objects/CIP_0002_MessageRouter/CIP_MessageRouter.hpp"
#include "network/NET_Endianconv.hpp"
#include "network/ethIP/eip_endianconv.hpp"

//Static variables
CIP_CommonPacket::PacketFormat CIP_CommonPacket::common_packet_data;


//Methods

int CIP_CommonPacket::NotifyCommonPacketFormat(EncapsulationData* recv_data, CipUsint* reply_buffer)
{
    CipStatus return_value;
    return_value.status = kCipStatusError;

    if ((return_value = CreateCommonPacketFormatStructure(recv_data->current_communication_buffer_position, recv_data->data_length, &common_packet_data)).status == kCipStatusError)
    {
        OPENER_TRACE_ERR("notifyCPF: error from createCPFstructure\n");
        return return_value.extended_status;
    }

    // In cases of errors we normally need to send an error response
    return_value.status = kCipStatusOk;
    // check if NullAddressItem received, otherwise it is no unconnected message and should not be here
    if (common_packet_data.address_item.type_id != kCipItemIdNullAddress)
    {
        OPENER_TRACE_ERR(
                "notifyCPF: got something besides the expected CIP_ITEM_ID_nullptr\n");
        recv_data->status = kEncapsulationProtocolIncorrectData;
        return return_value.extended_status;
    }

    // found null address item
    if (common_packet_data.data_item.type_id != kCipItemIdUnconnectedDataItem)
    {
        // wrong data item detected
        OPENER_TRACE_ERR("notifyCPF: got something besides the expected CIP_ITEM_ID_UNCONNECTEDMESSAGE\n");
        recv_data->status = kEncapsulationProtocolIncorrectData;
        return return_value.extended_status;
    }

    // unconnected data item received
    return_value = CIP_MessageRouter::NotifyMR(common_packet_data.data_item.data, common_packet_data.data_item.length);
    if (return_value.status == kCipStatusError)
    {
        return return_value.extended_status;
    }

    return_value.extended_status = (CipUdint) AssembleLinearMessage(&CIP_MessageRouter::g_message_router_response, &common_packet_data, reply_buffer);
    return return_value.extended_status;

}

int CIP_CommonPacket::NotifyConnectedCommonPacketFormat(EncapsulationData* recv_data, CipUsint* reply_buffer)
{

    CipStatus return_value = CreateCommonPacketFormatStructure(recv_data->current_communication_buffer_position, recv_data->data_length, &common_packet_data);

    if (kCipStatusError == return_value.status)
    {
        OPENER_TRACE_ERR("notifyConnectedCPF: error from createCPFstructure\n");
        return return_value.status;
    }

    // For connected explicit messages status always has to be 0
    return_value.status = kCipStatusError;

    // check if ConnectedAddressItem received, otherwise it is no connected message and should not be here
    if (common_packet_data.address_item.type_id != kCipItemIdConnectionAddress)
    {
        OPENER_TRACE_ERR("notifyConnectedCPF: got something besides the expected CIP_ITEM_ID_nullptr\n");
        return return_value.status;
    }

    // ConnectedAddressItem item
    CIP_ConnectionManager* connection_manager_object = CIP_ConnectionManager::GetConnectionManagerObject(common_packet_data.address_item.data.connection_identifier);
    if (nullptr == connection_manager_object)
    {
        OPENER_TRACE_ERR("notifyConnectedCPF: connection with given ID could not be found\n");
        return return_value.status;
    }

    // reset the watchdog timer
    connection_manager_object->inactivity_watchdog_timer = (connection_manager_object->o_to_t_requested_packet_interval / 1000)
            << (2 + connection_manager_object->connection_timeout_multiplier);

    //TODO check connection id  and sequence count
    if (common_packet_data.data_item.type_id != kCipItemIdConnectedDataItem)
    {
        /* wrong data item detected*/
        OPENER_TRACE_ERR("notifyConnectedCPF: got something besides the expected CIP_ITEM_ID_UNCONNECTEDMESSAGE\n");
        return return_value.status;
    }

    // connected data item received
    CipUsint* pnBuf = common_packet_data.data_item.data;
    common_packet_data.address_item.data.sequence_number = (CipUdint)NET_Endianconv::GetIntFromMessage(&pnBuf);
    return_value = CIP_MessageRouter::NotifyMR(pnBuf, common_packet_data.data_item.length - 2);

    if (return_value.status != kCipStatusError)
    {
        CIP_Connection * connection_object =
                connection_manager_object->producing_instance->id == common_packet_data.address_item.data.connection_identifier
                ? connection_manager_object->producing_instance : connection_manager_object->consuming_instance;

        common_packet_data.address_item.data.connection_identifier = connection_object->CIP_produced_connection_id;
        return_value = AssembleLinearMessage(&CIP_MessageRouter::g_message_router_response, &common_packet_data, reply_buffer);
    }

    return return_value.status;
}

/**
 * @brief Creates Common Packet Format structure out of data.
 * @param data Pointer to data which need to be structured.
 * @param data_length	Length of data in pa_Data.
 * @param common_packet_format_data	Pointer to structure of CPF data item.
 *
 *   @return kCipStatusOk .. success
 * 	       kCipStatusError .. error
 */
CipStatus CIP_CommonPacket::CreateCommonPacketFormatStructure(
    CipUsint* data, int data_length,
    PacketFormat* common_packet_format_data)
{


    common_packet_format_data->address_info_item[0].type_id = 0;
    common_packet_format_data->address_info_item[1].type_id = 0;

    int length_count = 0;
    common_packet_format_data->item_count = NET_Endianconv::GetIntFromMessage(&data);
    length_count += 2;
    if (common_packet_format_data->item_count >= 1)
    {
        common_packet_format_data->address_item.type_id = NET_Endianconv::GetIntFromMessage(&data);
        common_packet_format_data->address_item.length = NET_Endianconv::GetIntFromMessage(&data);
        length_count += 4;
        if (common_packet_format_data->address_item.length >= 4)
        {
            common_packet_format_data->address_item.data.connection_identifier = NET_Endianconv::GetDintFromMessage(&data);
            length_count += 4;
        }
        if (common_packet_format_data->address_item.length == 8)
        {
            common_packet_format_data->address_item.data.sequence_number = NET_Endianconv::GetDintFromMessage(&data);
            length_count += 4;
        }
    }
    if (common_packet_format_data->item_count >= 2)
    {
        common_packet_format_data->data_item.type_id = NET_Endianconv::GetIntFromMessage(&data);
        common_packet_format_data->data_item.length = NET_Endianconv::GetIntFromMessage(&data);
        common_packet_format_data->data_item.data = data;
        data += common_packet_format_data->data_item.length;
        length_count += (4 + common_packet_format_data->data_item.length);
    }
    for (int j = 0; j < (common_packet_format_data->item_count - 2); j++) // TODO there needs to be a limit check here???
    {
        common_packet_format_data->address_info_item[j].type_id = NET_Endianconv::GetIntFromMessage(&data);
        length_count += 2;
        if ((common_packet_format_data->address_info_item[j].type_id == kCipItemIdSocketAddressInfoOriginatorToTarget) || (common_packet_format_data->address_info_item[j].type_id == kCipItemIdSocketAddressInfoTargetToOriginator))
        {
            common_packet_format_data->address_info_item[j].length = NET_Endianconv::GetIntFromMessage(&data);
            common_packet_format_data->address_info_item[j].sin_family = NET_Endianconv::GetIntFromMessage(&data);
            common_packet_format_data->address_info_item[j].sin_port = NET_Endianconv::GetIntFromMessage(&data);
            common_packet_format_data->address_info_item[j].sin_addr = NET_Endianconv::GetDintFromMessage(&data);
            for (int i = 0; i < 8; i++)
            {
                common_packet_format_data->address_info_item[j].nasin_zero[i] = *data;
                data++;
            }
            length_count += 18;
        }
        else
        {
            // no sockaddr item found
            common_packet_format_data->address_info_item[j].type_id = 0; // mark as not set
            data -= 2;
        }
    }
    // set the addressInfoItems to not set if they were not received
    if (common_packet_format_data->item_count < 4) {
        common_packet_format_data->address_info_item[1].type_id = 0;
        if (common_packet_format_data->item_count < 3) {
            common_packet_format_data->address_info_item[0].type_id = 0;
        }
    }
    if (length_count == data_length)
    {
        // length of data is equal to length of Addr and length of Data
        return kCipStatusOk;
    }
    else
    {
        OPENER_TRACE_WARN("something is wrong with the length in Message Router @ CreateCommonPacketFormatStructure\n");
        if (common_packet_format_data->item_count > 2)
        {
            // there is an optional packet in data stream which is not sockaddr item
            return kCipStatusOk;
        }
        else
        {
            // something with the length was wrong
            return kCipStatusError;
        }
    }
}

// null address item -> address length set to 0
/**
 * Encodes a Null Address Item into the message frame
 * @param message The message frame
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeNullAddressItem(CipUsint** message, int size)
{
    // null address item -> address length set to 0
    size += NET_Endianconv::AddIntToMessage(kCipItemIdNullAddress, message);
    size += NET_Endianconv::AddIntToMessage(0, message);
    return size;
}

// connected data item -> address length set to 4 and copy ConnectionIdentifier
/**
 * Encodes a Connected Address Item into the message frame
 * @param message The message frame
 * @param common_packet_format_data_item The Common Packet Format data structure from which the message is constructed
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeConnectedAddressItem(CipUsint** message, PacketFormat* common_packet_format_data_item, int size)
{
    // connected data item -> address length set to 4 and copy ConnectionIdentifier
    size += NET_Endianconv::AddIntToMessage(kCipItemIdConnectionAddress, message);
    size += NET_Endianconv::AddIntToMessage(4, message);
    size += NET_Endianconv::AddDintToMessage(common_packet_format_data_item->address_item.data.connection_identifier, message);
    return size;
}

// TODO: Add doxygen documentation
// sequenced address item -> address length set to 8 and copy ConnectionIdentifier and SequenceNumber
// sequence number?????
int CIP_CommonPacket::EncodeSequencedAddressItem(CipUsint** message, PacketFormat* common_packet_format_data_item, int size)
{
    // sequenced address item -> address length set to 8 and copy ConnectionIdentifier and SequenceNumber
    size += NET_Endianconv::AddIntToMessage(kCipItemIdSequencedAddressItem, message);
    size += NET_Endianconv::AddIntToMessage(8, message);
    size += NET_Endianconv::AddDintToMessage(common_packet_format_data_item->address_item.data.connection_identifier, message);
    size += NET_Endianconv::AddDintToMessage(common_packet_format_data_item->address_item.data.sequence_number, message);
    return size;
}

/**
 * Adds the item count to the message frame
 *
 * @param common_packet_format_data_item The Common Packet Format data structure from which the message is constructed
 * @param message The message frame
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeItemCount(PacketFormat* common_packet_format_data_item, CipUsint** message, int size)
{
    size += NET_Endianconv::AddIntToMessage(common_packet_format_data_item->item_count, message); // item count
    return size;
}

/**
 * Adds the data item type to the message frame
 *
 * @param common_packet_format_data_item The Common Packet Format data structure from which the message is constructed
 * @param message The message frame
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeDataItemType(PacketFormat* common_packet_format_data_item, CipUsint** message, int size)
{
    size += NET_Endianconv::AddIntToMessage(common_packet_format_data_item->data_item.type_id, message);
    return size;
}

/**
 * Adds the data item section length to the message frame
 *
 * @param common_packet_format_data_item The Common Packet Format data structure from which the message is constructed
 * @param message The message frame
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeDataItemLength(PacketFormat* common_packet_format_data_item, CipUsint** message, int size)
{
    size += NET_Endianconv::AddIntToMessage(common_packet_format_data_item->data_item.length, message);
    return size;
}

/**
 * Adds the data items to the message frame
 *
 * @param common_packet_format_data_item The Common Packet Format data structure from which the message is constructed
 * @param message The message frame
 * @param size The actual size of the message frame
 *
 * @return The new size of the message frame after encoding
 */
int CIP_CommonPacket::EncodeDataItemData(PacketFormat* common_packet_format_data_item, CipUsint** message, int size)
{
    for (int i = 0; i < common_packet_format_data_item->data_item.length; i++)
    {
        size += NET_Endianconv::AddSintToMessage(*(common_packet_format_data_item->data_item.data + i), message);
    }
    return size;
}

int CIP_CommonPacket::EncodeConnectedDataItemLength(CipMessageRouterResponse_t* message_router_response, CipUsint** message, int size)
{//todo:recheck
    size += NET_Endianconv::AddIntToMessage((CipUint)(message_router_response->size_additional_status + 4 + 2 + (2 * message_router_response->size_additional_status)), message);
    return size;
}

int CIP_CommonPacket::EncodeSequenceNumber(int size, const PacketFormat* common_packet_format_data_item, CipUsint** message)
{
    // 2 bytes
    size += NET_Endianconv::AddIntToMessage((CipUint)common_packet_format_data_item->address_item.data.sequence_number, message);
    return size;
}

int CIP_CommonPacket::EncodeReplyService(int size, CipUsint** message, CipMessageRouterResponse_t* message_router_response)
{
    size += NET_Endianconv::AddSintToMessage(message_router_response->reply_service, message);
    return size;
}

int CIP_CommonPacket::EncodeReservedFieldOfLengthByte(int size, CipUsint** message, CipMessageRouterResponse_t* message_router_response)
{
    size += NET_Endianconv::AddSintToMessage(message_router_response->reserved, message);
    return size;
}

int CIP_CommonPacket::EncodeGeneralStatus(int size, CipUsint** message, CipMessageRouterResponse_t* message_router_response)
{
    size += NET_Endianconv::AddSintToMessage(message_router_response->general_status, message);
    return size;
}

int CIP_CommonPacket::EncodeExtendedStatusLength(int size, CipUsint** message, CipMessageRouterResponse_t* message_router_response)
{
    size += NET_Endianconv::AddSintToMessage(message_router_response->size_additional_status, message);
    return size;
}

int CIP_CommonPacket::EncodeExtendedStatusDataItems(int size, CipMessageRouterResponse_t* message_router_response, CipUsint** message)
{
    for (int i = 0; i < message_router_response->size_additional_status; i++)
        size += NET_Endianconv::AddIntToMessage(message_router_response->additional_status[i], message);

    return size;
}

int CIP_CommonPacket::EncodeExtendedStatus(int size, CipUsint** message, CipMessageRouterResponse_t* message_router_response)
{
    size = EncodeExtendedStatusLength(size, message, message_router_response);
    size = EncodeExtendedStatusDataItems(size, message_router_response, message);

    return size;
}

int CIP_CommonPacket::EncodeUnconnectedDataItemLength(int size, CipMessageRouterResponse_t* message_router_response, CipUsint** message)
{
    // Unconnected Item //todo:recheck
    size += NET_Endianconv::AddIntToMessage((CipUint)(message_router_response->size_additional_status + 4 + (2 * message_router_response->size_additional_status)), message);
    return size;
}

int CIP_CommonPacket::EncodeMessageRouterResponseData(int size, CipMessageRouterResponse_t* message_router_response, CipUsint** message)
{
    //todo:recheck
    for (int i = 0; i < message_router_response->size_additional_status; i++)
    {
        size += NET_Endianconv::AddSintToMessage((message_router_response->additional_status)[i], &*message);
    }
    return size;
}

int CIP_CommonPacket::EncodeSockaddrInfoItemTypeId(int size, int item_type, PacketFormat* common_packet_format_data_item, CipUsint** message)
{
    OPENER_ASSERT(item_type == 0 || item_type == 1);
    size += NET_Endianconv::AddIntToMessage(common_packet_format_data_item->address_info_item[item_type].type_id, message);

    return size;
}

int CIP_CommonPacket::EncodeSockaddrInfoLength(int size, int j, PacketFormat* common_packet_format_data_item, CipUsint** message)
{
    size += NET_Endianconv::AddIntToMessage(common_packet_format_data_item->address_info_item[j].length, message);
    return size;
}

/** @brief Copy data from message_router_response struct and common_packet_format_data_item into linear memory in
 * pa_msg for transmission over in encapsulation.
 *
 * @param message_router_response	pointer to message router response which has to be aligned into linear memory.
 * @param common_packet_format_data_item pointer to CPF structure which has to be aligned into linear memory.
 * @param message		pointer to linear memory.
 *  @return length of reply in message in bytes
 * 			-1 .. error
 */
int CIP_CommonPacket::AssembleLinearMessage(CipMessageRouterResponse_t* message_router_response, PacketFormat* common_packet_format_data_item, CipUsint* message)
{

    int message_size = 0;

    if (message_router_response)
    {
        // add Interface Handle and Timeout = 0 -> only for SendRRData and SendUnitData necessary
        NET_Endianconv::AddDintToMessage(0, &message);
        NET_Endianconv::AddIntToMessage(0, &message);
        message_size += 6;
    }

    message_size = EncodeItemCount(common_packet_format_data_item, &message, message_size);

    // process Address Item
    switch (common_packet_format_data_item->address_item.type_id)
    {
        case kCipItemIdNullAddress:
            message_size = EncodeNullAddressItem(&message, message_size);
            break;
        case kCipItemIdConnectionAddress:
            message_size = EncodeConnectedAddressItem(&message, common_packet_format_data_item, message_size);
            break;
        case kCipItemIdSequencedAddressItem:
            message_size = EncodeSequencedAddressItem(&message, common_packet_format_data_item, message_size);
            break;
        default:
            break;
    }

    // process Data Item
    if ((common_packet_format_data_item->data_item.type_id == kCipItemIdUnconnectedDataItem) || (common_packet_format_data_item->data_item.type_id == kCipItemIdConnectedDataItem))
    {

        if (message_router_response)
        {
            message_size = EncodeDataItemType(common_packet_format_data_item, &message, message_size);

            if (common_packet_format_data_item->data_item.type_id == kCipItemIdConnectedDataItem)
            {
                //Connected Item
                message_size = EncodeConnectedDataItemLength(message_router_response, &message, message_size);
                message_size = EncodeSequenceNumber(message_size, &common_packet_data, &message);

            }
            else
            {
                // Unconnected Item
                message_size = EncodeUnconnectedDataItemLength(message_size, message_router_response, &message);
            }

            // write message router response into linear memory
            message_size = EncodeReplyService(message_size, &message, message_router_response);
            message_size = EncodeReservedFieldOfLengthByte(message_size, &message, message_router_response);
            message_size = EncodeGeneralStatus(message_size, &message, message_router_response);
            message_size = EncodeExtendedStatus(message_size, &message, message_router_response);
            message_size = EncodeMessageRouterResponseData(message_size,message_router_response, &message);
        }
        else
        {
            // connected IO Message to send
            message_size = EncodeDataItemType(common_packet_format_data_item, &message, message_size);
            message_size = EncodeDataItemLength(common_packet_format_data_item, &message, message_size);
            message_size = EncodeDataItemData(common_packet_format_data_item, &message, message_size);
        }
    }

    // process SockAddr Info Items
    // make sure first the O->T and then T->O appears on the wire.
    // EtherNet/IP specification doesn't demand it, but there are EIP
    // devices which depend on CPF items to appear in the order of their
    // ID number
    for (int type = kCipItemIdSocketAddressInfoOriginatorToTarget; type <= kCipItemIdSocketAddressInfoTargetToOriginator; type++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (common_packet_format_data_item->address_info_item[j].type_id == type)
            {
                message_size = EncodeSockaddrInfoItemTypeId(message_size, j, common_packet_format_data_item, &message);

                message_size = EncodeSockaddrInfoLength(message_size, j, common_packet_format_data_item, &message);

                message_size += EncapsulateIpAddress(common_packet_format_data_item->address_info_item[j].sin_port, common_packet_format_data_item->address_info_item[j].sin_addr, &message);

                message_size += NET_Endianconv::FillNextNMessageOctetsWithValueAndMoveToNextPosition(0, 8, &message);
                break;
            }
        }
    }
    return message_size;
}
int CIP_CommonPacket::AssembleIOMessage(PacketFormat* common_packet_format_data_item, CipUsint* message)
{
    return AssembleLinearMessage(0, common_packet_format_data_item, (CipUsint*)CIP_Common::message_data_reply_buffer[0]);
}
