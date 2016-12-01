/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <string.h>
#include <cstdlib>

#include "src/cip/connection_stack/CIP_Common.h"
#include "src/cip/connection_stack/CIP_Connection.h"
#include "CIP_Identity.h"
#include "src/cip/connection_stack/CIP_MessageRouter.h"
#include "CIP_CommonPacket.h"
#include "eip_encap.h"
#include "eip_endianconv.h"
#include "endianconv.h"
#include "src/cip/network_stack/NET_NetworkHandler.h"
#include "Opener_Interface.h"

/*Identity data from cipidentity.c*/
extern CipUint vendor_id_;
extern CipUint device_type_;
extern CipUint product_code_;
extern CipRevision revision_;
extern CipUint status_;
extern CipUdint serial_number_;
extern CipShortString product_name_;

/*ip address data taken from TCPIPInterfaceObject*/
extern CipTcpIpNetworkInterfaceConfiguration interface_configuration_;

const int kSupportedProtocolVersion = 1; /**< Supported Encapsulation protocol version */

const int kEncapsulationHeaderOptionsFlag = 0x00; /**< Mask of which options are supported as of the current CIP specs no other option value as 0 should be supported.*/

const int kEncapsulationHeaderSessionHandlePosition = 4; /**< the position of the session handle within the encapsulation header*/

const int kListIdentityDefaultDelayTime = 2000; /**< Default delay time for List Identity response */
const int kListIdentityMinimumDelayTime = 500; /**< Minimum delay time for List Identity response */

typedef enum {
    kSessionStatusInvalid = -1,
    kSessionStatusValid = 0
} SessionStatus;

const int kSenderContextSize = 8; /**< size of sender context in encapsulation header*/

/** @brief definition of known encapsulation commands */
typedef enum {
    kEncapsulationCommandNoOperation = 0x0000, /**< only allowed for TCP */
    kEncapsulationCommandListServices = 0x0004, /**< allowed for both UDP and TCP */
    kEncapsulationCommandListIdentity = 0x0063, /**< allowed for both UDP and TCP */
    kEncapsulationCommandListInterfaces = 0x0064, /**< optional, allowed for both UDP and TCP */
    kEncapsulationCommandRegisterSession = 0x0065, /**< only allowed for TCP */
    kEncapsulationCommandUnregisterSession = 0x0066, /**< only allowed for TCP */
    kEncapsulationCommandSendRequestReplyData = 0x006F, /**< only allowed for TCP */
    kEncapsulationCommandSendUnitData = 0x0070 /**< only allowed for TCP */
} EncapsulationCommand;

/** @brief definition of capability flags */
typedef enum {
    kCapabilityFlagsCipTcp = 0x0020,
    kCapabilityFlagsCipUdpClass0or1 = 0x0100
} CapabilityFlags;

#define ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES 2 /**< According to EIP spec at least 2 delayed message requests should be supported */

#define ENCAP_MAX_DELAYED_ENCAP_MESSAGE_SIZE (ENCAPSULATION_HEADER_LENGTH + 39 + sizeof(OPENER_DEVICE_NAME)) /* currently we only have the size of an encapsulation message */

/* Encapsulation layer data  */

/** @brief Delayed Encapsulation Message structure */
typedef struct {
    CipDint time_out; /**< time out in milli seconds */
    int socket; /**< associated socket */
    struct sockaddr_in receiver;
    CipByte message[ENCAP_MAX_DELAYED_ENCAP_MESSAGE_SIZE];
    unsigned int message_size;
} DelayedEncapsulationMessage;

EncapsulationInterfaceInformation g_interface_information;

int g_registered_sessions[OPENER_NUMBER_OF_SUPPORTED_SESSIONS];

DelayedEncapsulationMessage g_delayed_encapsulation_messages[ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES];

/*** private functions ***/
void HandleReceivedListServicesCommand(EncapsulationData* receive_data);

void HandleReceivedListInterfacesCommand(EncapsulationData* receive_data);

void HandleReceivedListIdentityCommandTcp(EncapsulationData* receive_data);

void HandleReceivedListIdentityCommandUdp(int socket,
    struct sockaddr_in* from_address,
    EncapsulationData* receive_data);

void HandleReceivedRegisterSessionCommand(int socket,
    EncapsulationData* receive_data);

CipStatus HandleReceivedUnregisterSessionCommand(
    EncapsulationData* receive_data);

CipStatus HandleReceivedSendUnitDataCommand(EncapsulationData* receive_data);

CipStatus HandleReceivedSendRequestResponseDataCommand(
    EncapsulationData* receive_data);

int GetFreeSessionIndex(void);

CipInt CreateEncapsulationStructure(CipUsint* receive_buffer,
    int receive_buffer_length,
    EncapsulationData* encapsulation_data);

SessionStatus CheckRegisteredSessions(EncapsulationData* receive_data);

int EncapsulateData(const EncapsulationData* const send_data);

void DetermineDelayTime(CipByte* buffer_start,
    DelayedEncapsulationMessage* delayed_message_buffer);

int EncapsulateListIdentyResponseMessage(CipByte* const communication_buffer);

/*   @brief Initializes session list and interface information. */
void EncapsulationInit(void)
{

    DetermineEndianess();

    /*initialize random numbers for random delayed response message generation
   * we use the ip address as seed as suggested in the spec */
    srand(interface_configuration_.ip_address);

    /* initialize Sessions to invalid == free session */
    for (unsigned int i = 0; i < OPENER_NUMBER_OF_SUPPORTED_SESSIONS; i++)
    {
        g_registered_sessions[i] = kEipInvalidSocket;
    }

    for (unsigned int i = 0; i < ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES; i++)
    {
        g_delayed_encapsulation_messages[i].socket = -1;
    }

    /*TODO make the interface information configurable*/
    /* initialize interface information */
    g_interface_information.type_code = CIP_CommonPacket::kCipItemIdListServiceResponse;
    g_interface_information.length = sizeof(g_interface_information);
    g_interface_information.encapsulation_protocol_version = 1;
    g_interface_information.capability_flags = kCapabilityFlagsCipTcp | kCapabilityFlagsCipUdpClass0or1;
    strcpy((char*)g_interface_information.name_of_service, "Communications");
}

int HandleReceivedExplictTcpData(int socket, CipUsint* buffer,
    unsigned int length, int* remaining_bytes)
{
    CipStatus return_value = kCipStatusOk;
    EncapsulationData encapsulation_data;
    /* eat the encapsulation header*/
    /* the structure contains a pointer to the encapsulated data*/
    /* returns how many bytes are left after the encapsulated data*/
    *remaining_bytes = CreateEncapsulationStructure(buffer, length, &encapsulation_data);

    if (kEncapsulationHeaderOptionsFlag == encapsulation_data.options) /*TODO generate appropriate error response*/
    {
        if (*remaining_bytes >= 0) /* check if the message is corrupt: header size + claimed payload size > than what we actually received*/
        {
            /* full package or more received */
            encapsulation_data.status = kEncapsulationProtocolSuccess;
            return_value = kCipStatusOkSend;
            /* most of these functions need a reply to be send */
            switch (encapsulation_data.command_code) {
                case (kEncapsulationCommandNoOperation):
                    /* NOP needs no reply and does nothing */
                    return_value = kCipStatusOk;
                    break;

                case (kEncapsulationCommandListServices):
                    HandleReceivedListServicesCommand(&encapsulation_data);
                    break;

                case (kEncapsulationCommandListIdentity):
                    HandleReceivedListIdentityCommandTcp(&encapsulation_data);
                    break;

                case (kEncapsulationCommandListInterfaces):
                    HandleReceivedListInterfacesCommand(&encapsulation_data);
                    break;

                case (kEncapsulationCommandRegisterSession):
                    HandleReceivedRegisterSessionCommand(socket, &encapsulation_data);
                    break;

                case (kEncapsulationCommandUnregisterSession):
                    return_value = HandleReceivedUnregisterSessionCommand(
                        &encapsulation_data);
                    break;

                case (kEncapsulationCommandSendRequestReplyData):
                    return_value = HandleReceivedSendRequestResponseDataCommand(
                        &encapsulation_data);
                    break;

                case (kEncapsulationCommandSendUnitData):
                    return_value = HandleReceivedSendUnitDataCommand(&encapsulation_data);
                    break;

                default:
                    encapsulation_data.status = kEncapsulationProtocolInvalidCommand;
                    encapsulation_data.data_length = 0;
                    break;
            }
            /* if nRetVal is greater than 0 data has to be sent */
            if (kCipStatusOk < return_value)
            {
                return_value = (CipStatus)EncapsulateData(&encapsulation_data);
            }
        }
    }

    return return_value;
}

int HandleReceivedExplictUdpData(int socket, struct sockaddr_in* from_address,
    CipUsint* buffer, unsigned int buffer_length, int* number_of_remaining_bytes, int unicast)
{
    CipStatus status = kCipStatusOk;
    EncapsulationData encapsulation_data;
    /* eat the encapsulation header*/
    /* the structure contains a pointer to the encapsulated data*/
    /* returns how many bytes are left after the encapsulated data*/
    *number_of_remaining_bytes = CreateEncapsulationStructure(buffer, buffer_length, &encapsulation_data);

    if (kEncapsulationHeaderOptionsFlag == encapsulation_data.options) /*TODO generate appropriate error response*/
    {
        if (*number_of_remaining_bytes >= 0) /* check if the message is corrupt: header size + claimed payload size > than what we actually received*/
        {
            /* full package or more received */
            encapsulation_data.status = kEncapsulationProtocolSuccess;
            status = kCipStatusOkSend;
            /* most of these functions need a reply to be send */
            switch (encapsulation_data.command_code)
            {
                case (kEncapsulationCommandListServices):
                    HandleReceivedListServicesCommand(&encapsulation_data);
                    break;

                case (kEncapsulationCommandListIdentity):
                    if (unicast == true)
                    {
                        HandleReceivedListIdentityCommandTcp(&encapsulation_data);
                    } else
                    {
                        HandleReceivedListIdentityCommandUdp(socket, from_address,
                            &encapsulation_data);
                        status = kCipStatusOk;
                    } /* as the response has to be delayed do not send it now */
                    break;

                case (kEncapsulationCommandListInterfaces):
                    HandleReceivedListInterfacesCommand(&encapsulation_data);
                    break;

                /* The following commands are not to be sent via UDP */
                case (kEncapsulationCommandNoOperation):
                case (kEncapsulationCommandRegisterSession):
                case (kEncapsulationCommandUnregisterSession):
                case (kEncapsulationCommandSendRequestReplyData):
                case (kEncapsulationCommandSendUnitData):
                default:
                    encapsulation_data.status = kEncapsulationProtocolInvalidCommand;
                    encapsulation_data.data_length = 0;
                    break;
            }
            /* if nRetVal is greater than 0 data has to be sent */
            if (0 < status)
            {
                status = (CipStatus)EncapsulateData(&encapsulation_data);
            }
        }
    }
    return status;
}

int EncapsulateData(const EncapsulationData* const send_data)
{
    CipUsint* communcation_buffer = send_data->communication_buffer_start + 2;
    AddIntToMessage(send_data->data_length, &communcation_buffer);
    /*the CommBuf should already contain the correct session handle*/
    MoveMessageNOctets(4, &communcation_buffer);
    AddDintToMessage(send_data->status, &communcation_buffer);
    /*the CommBuf should already contain the correct sender context*/
    /*the CommBuf should already contain the correct  options value*/

    return ENCAPSULATION_HEADER_LENGTH + send_data->data_length;
}

/** @brief generate reply with "Communications Services" + compatibility Flags.
 *  @param receive_data pointer to structure with received data
 */
void HandleReceivedListServicesCommand(EncapsulationData* receive_data)
{
    CipUsint* communication_buffer = receive_data->current_communication_buffer_position;

    receive_data->data_length = g_interface_information.length + 2;

    /* copy Interface data to msg for sending */
    AddIntToMessage(1, &communication_buffer);

    AddIntToMessage(g_interface_information.type_code, &communication_buffer);

    AddIntToMessage((CipUint)(g_interface_information.length - 4), &communication_buffer);

    AddIntToMessage(g_interface_information.encapsulation_protocol_version, &communication_buffer);

    AddIntToMessage(g_interface_information.capability_flags, &communication_buffer);

    memcpy(communication_buffer, g_interface_information.name_of_service, sizeof(g_interface_information.name_of_service));
}

void HandleReceivedListInterfacesCommand(EncapsulationData* receive_data)
{
    CipUsint* communication_buffer = receive_data->current_communication_buffer_position;
    receive_data->data_length = 2;
    AddIntToMessage(0x0000, &communication_buffer); /* copy Interface data to msg for sending */
}

void HandleReceivedListIdentityCommandTcp(EncapsulationData* receive_data)
{
    receive_data->data_length = EncapsulateListIdentyResponseMessage(receive_data->current_communication_buffer_position);
}

void HandleReceivedListIdentityCommandUdp(int socket,
    struct sockaddr_in* from_address,
    EncapsulationData* receive_data)
{
    DelayedEncapsulationMessage* delayed_message_buffer = NULL;

    for (unsigned int i = 0; i < ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES; i++)
    {
        if (kEipInvalidSocket == g_delayed_encapsulation_messages[i].socket)
        {
            delayed_message_buffer = &(g_delayed_encapsulation_messages[i]);
            break;
        }
    }

    if (NULL != delayed_message_buffer)
    {
        delayed_message_buffer->socket = socket;
        memcpy((&delayed_message_buffer->receiver), from_address, sizeof(struct sockaddr_in));

        DetermineDelayTime(receive_data->communication_buffer_start, delayed_message_buffer);

        memcpy(&(delayed_message_buffer->message[0]), receive_data->communication_buffer_start, ENCAPSULATION_HEADER_LENGTH);

        delayed_message_buffer->message_size = EncapsulateListIdentyResponseMessage(
            &(delayed_message_buffer->message[ENCAPSULATION_HEADER_LENGTH]));

        CipUsint* communication_buffer = delayed_message_buffer->message + 2;
        AddIntToMessage(delayed_message_buffer->message_size, &communication_buffer);
        delayed_message_buffer->message_size += ENCAPSULATION_HEADER_LENGTH;
    }
}

int EncapsulateListIdentyResponseMessage(CipByte* const communication_buffer)
{
    CipUsint* communication_buffer_runner = communication_buffer;

    AddIntToMessage(1, &(communication_buffer_runner)); /* Item count: one item */
    AddIntToMessage(CIP_CommonPacket::kCipItemIdListIdentityResponse, &communication_buffer_runner);

    CipByte* id_length_buffer = communication_buffer_runner;
    communication_buffer_runner += 2; /*at this place the real length will be inserted below*/

    AddIntToMessage(kSupportedProtocolVersion, &communication_buffer_runner);

    EncapsulateIpAddress(htons(kOpenerEthernetPort), interface_configuration_.ip_address, &communication_buffer_runner);

    memset(communication_buffer_runner, 0, 8);

    communication_buffer_runner += 8;

    AddIntToMessage(vendor_id_, &communication_buffer_runner);

    AddIntToMessage(device_type_, &communication_buffer_runner);

    AddIntToMessage(product_code_, &communication_buffer_runner);

    *(communication_buffer_runner)++ = revision_.major_revision;

    *(communication_buffer_runner)++ = revision_.minor_revision;

    AddIntToMessage(status_, &communication_buffer_runner);

    AddDintToMessage(serial_number_, &communication_buffer_runner);

    *communication_buffer_runner++ = (unsigned char)product_name_.length;

    memcpy(communication_buffer_runner, product_name_.string, product_name_.length);

    communication_buffer_runner += product_name_.length;

    *communication_buffer_runner++ = 0xFF;

    // the -2 is for not counting the length field
    AddIntToMessage(communication_buffer_runner - id_length_buffer - 2, &id_length_buffer);

    return communication_buffer_runner - communication_buffer;
}

void DetermineDelayTime(CipByte* buffer_start,  DelayedEncapsulationMessage* delayed_message_buffer)
{

    buffer_start += 12; /* start of the sender context */
    CipUint maximum_delay_time = GetIntFromMessage(&buffer_start);

    if (0 == maximum_delay_time)
    {
        maximum_delay_time = kListIdentityDefaultDelayTime;
    }
    else if (kListIdentityMinimumDelayTime > maximum_delay_time)
    {
        // if maximum_delay_time is between 1 and 500ms set it to 500ms
        maximum_delay_time = kListIdentityMinimumDelayTime;
    }

    // Sets delay time between 0 and maximum_delay_time
    delayed_message_buffer->time_out = (maximum_delay_time * std::rand()) / RAND_MAX;
}

/* @brief Check supported protocol, generate session handle, send replay back to originator.
 * @param socket Socket this request is associated to. Needed for double register check
 * @param receive_data Pointer to received data with request/response.
 */
void HandleReceivedRegisterSessionCommand(int socket,
    EncapsulationData* receive_data)
{
    int session_index = 0;
    CipUsint* receive_data_buffer;
    CipUint protocol_version = GetIntFromMessage(
        &receive_data->current_communication_buffer_position);
    CipUint nOptionFlag = GetIntFromMessage(
        &receive_data->current_communication_buffer_position);

    /* check if requested protocol version is supported and the register session option flag is zero*/
    if ((0 < protocol_version) && (protocol_version <= kSupportedProtocolVersion)
        && (0 == nOptionFlag)) { /*Option field should be zero*/
        /* check if the socket has already a session open */
        for (int i = 0; i < OPENER_NUMBER_OF_SUPPORTED_SESSIONS; ++i) {
            if (g_registered_sessions[i] == socket) {
                /* the socket has already registered a session this is not allowed*/
                receive_data->session_handle = i + 1; /*return the already assigned session back, the cip spec is not clear about this needs to be tested*/
                receive_data->status = kEncapsulationProtocolInvalidCommand;
                session_index = kSessionStatusInvalid;
                receive_data_buffer = &receive_data->communication_buffer_start[kEncapsulationHeaderSessionHandlePosition];
                AddDintToMessage(receive_data->session_handle, &receive_data_buffer); /*EncapsulateData will not update the session handle so we have to do it here by hand*/
                break;
            }
        }

        if (kSessionStatusInvalid != session_index) {
            session_index = GetFreeSessionIndex();
            if (kSessionStatusInvalid == session_index) /* no more sessions available */
            {
                receive_data->status = kEncapsulationProtocolInsufficientMemory;
            } else { /* successful session registered */
                g_registered_sessions[session_index] = socket; /* store associated socket */
                receive_data->session_handle = session_index + 1;
                receive_data->status = kEncapsulationProtocolSuccess;
                receive_data_buffer = &receive_data->communication_buffer_start[kEncapsulationHeaderSessionHandlePosition];
                AddDintToMessage(receive_data->session_handle, &receive_data_buffer); /*EncapsulateData will not update the session handle so we have to do it here by hand*/
            }
        }
    } else { /* protocol not supported */
        receive_data->status = kEncapsulationProtocolUnsupportedProtocol;
    }

    receive_data->data_length = 4;
}

/*   INT8 UnregisterSession(struct S_Encapsulation_Data *pa_S_ReceiveData)
 *   close all corresponding TCP connections and delete session handle.
 *      pa_S_ReceiveData pointer to unregister session request with corresponding socket handle.
 */
CipStatus HandleReceivedUnregisterSessionCommand(
    EncapsulationData* receive_data)
{
    int i;

    if ((0 < receive_data->session_handle)
        && (receive_data->session_handle <= OPENER_NUMBER_OF_SUPPORTED_SESSIONS)) {
        i = receive_data->session_handle - 1;
        if (kEipInvalidSocket != g_registered_sessions[i]) {
            IApp_CloseSocket_tcp(g_registered_sessions[i]);
            g_registered_sessions[i] = kEipInvalidSocket;
            return kCipStatusOk;
        }
    }

    /* no such session registered */
    receive_data->data_length = 0;
    receive_data->status = kEncapsulationProtocolInvalidSessionHandle;
    return kCipStatusOkSend;
}

/** @brief Call Connection Manager.
 *  @param receive_data Pointer to structure with data and header information.
 */
CipStatus HandleReceivedSendUnitDataCommand(EncapsulationData* receive_data)
{
    CipInt send_size;
    CipStatus return_value = kCipStatusOkSend;

    if (receive_data->data_length >= 6) {
        /* Command specific data UDINT .. Interface Handle, UINT .. Timeout, CPF packets */
        /* don't use the data yet */
        GetDintFromMessage(&receive_data->current_communication_buffer_position); /* skip over null interface handle*/
        GetIntFromMessage(&receive_data->current_communication_buffer_position); /* skip over unused timeout value*/
        receive_data->data_length -= 6; /* the rest is in CPF format*/

        if (kSessionStatusValid == CheckRegisteredSessions(receive_data)) /* see if the EIP session is registered*/
        {
            send_size = CIP_CommonPacket::NotifyConnectedCommonPacketFormat(
                receive_data,
                &receive_data->communication_buffer_start[ENCAPSULATION_HEADER_LENGTH]);

            if (0 < send_size)
            { /* need to send reply */
                receive_data->data_length = send_size;
            }
            else
            {
                return_value = kCipStatusError;
            }
        }
        else
        { /* received a package with non registered session handle */
            receive_data->data_length = 0;
            receive_data->status = kEncapsulationProtocolInvalidSessionHandle;
        }
    }
    return return_value;
}

/** @brief Call UCMM or Message Router if UCMM not implemented.
 *  @param receive_data Pointer to structure with data and header information.
 *  @return status 	0 .. success.
 * 					-1 .. error
 */
CipStatus HandleReceivedSendRequestResponseDataCommand(
    EncapsulationData* receive_data)
{
    CipInt send_size;
    CipStatus return_value = kCipStatusOkSend;

    if (receive_data->data_length >= 6) {
        /* Command specific data UDINT .. Interface Handle, UINT .. Timeout, CPF packets */
        /* don't use the data yet */
        GetDintFromMessage(&receive_data->current_communication_buffer_position); /* skip over null interface handle*/
        GetIntFromMessage(&receive_data->current_communication_buffer_position); /* skip over unused timeout value*/
        receive_data->data_length -= 6; /* the rest is in CPF format*/

        if (kSessionStatusValid == CheckRegisteredSessions(receive_data)) /* see if the EIP session is registered*/
        {
            send_size = CIP_CommonPacket::NotifyCommonPacketFormat(
                receive_data,
                &receive_data->communication_buffer_start[ENCAPSULATION_HEADER_LENGTH]);

            if (send_size >= 0)
            {
                // need to send reply
                receive_data->data_length = send_size;
            }
            else
            {
                return_value = kCipStatusError;
            }
        }
        else
        {
            // received a package with non registered session handle
            receive_data->data_length = 0;
            receive_data->status = kEncapsulationProtocolInvalidSessionHandle;
        }
    }
    return return_value;
}

/** @brief search for available sessions an return index.
 *  @return return index of free session in anRegisteredSessions.
 * 			kInvalidSession .. no free session available
 */
int GetFreeSessionIndex(void)
{
    for (int session_index = 0; session_index < OPENER_NUMBER_OF_SUPPORTED_SESSIONS; session_index++)
    {
        if (kEipInvalidSocket == g_registered_sessions[session_index])
        {
            return session_index;
        }
    }
    return kSessionStatusInvalid;
}

/** @brief copy data from pa_buf in little endian to host in structure.
 * @param receive_buffer
 * @param length Length of the data in receive_buffer. Might be more than one message
 * @param encapsulation_data	structure to which data shall be copied
 * @return return difference between bytes in pa_buf an data_length
 *  		0 .. full package received
 * 			>0 .. more than one packet received
 * 			<0 .. only fragment of data portion received
 */
CipInt CreateEncapsulationStructure(CipUsint* receive_buffer, int receive_buffer_length, EncapsulationData* encapsulation_data)
{
    encapsulation_data->communication_buffer_start = receive_buffer;
    encapsulation_data->command_code = GetIntFromMessage(&receive_buffer);
    encapsulation_data->data_length = GetIntFromMessage(&receive_buffer);
    encapsulation_data->session_handle = GetDintFromMessage(&receive_buffer);
    encapsulation_data->status = GetDintFromMessage(&receive_buffer);

    receive_buffer += kSenderContextSize;
    encapsulation_data->options = GetDintFromMessage(&receive_buffer);
    encapsulation_data->current_communication_buffer_position = receive_buffer;
    return (receive_buffer_length - ENCAPSULATION_HEADER_LENGTH - encapsulation_data->data_length);
}

/** @brief Check if received package belongs to registered session.
 *  @param receive_data Received data.
 *  @return 0 .. Session registered
 *  		kInvalidSession .. invalid session -> return unsupported command received
 */
SessionStatus CheckRegisteredSessions(EncapsulationData* receive_data)
{
    if ((0 < receive_data->session_handle) && (receive_data->session_handle <= OPENER_NUMBER_OF_SUPPORTED_SESSIONS))
    {
        if (kEipInvalidSocket != g_registered_sessions[receive_data->session_handle - 1])
        {
            return kSessionStatusValid;
        }
    }
    return kSessionStatusInvalid;
}

void CloseSession(int socket)
{
    int i;
    for (i = 0; i < OPENER_NUMBER_OF_SUPPORTED_SESSIONS; ++i)
    {
        if (g_registered_sessions[i] == socket)
        {
            IApp_CloseSocket_tcp(socket);
            g_registered_sessions[i] = kEipInvalidSocket;
            break;
        }
    }
}

void EncapsulationShutDown(void)
{
    for (int i = 0; i < OPENER_NUMBER_OF_SUPPORTED_SESSIONS; ++i)
    {
        if (kEipInvalidSocket != g_registered_sessions[i])
        {
            IApp_CloseSocket_tcp(g_registered_sessions[i]);
            g_registered_sessions[i] = kEipInvalidSocket;
        }
    }
}

void ManageEncapsulationMessages(MilliSeconds elapsed_time)
{
    for (unsigned int i = 0; i < ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES; i++)
    {
        if (kEipInvalidSocket != g_delayed_encapsulation_messages[i].socket)
        {
            g_delayed_encapsulation_messages[i].time_out -= elapsed_time;
            if (0 > g_delayed_encapsulation_messages[i].time_out)
            {
                // If delay is reached or passed, send the UDP message
                SendUdpData(&(g_delayed_encapsulation_messages[i].receiver),
                    g_delayed_encapsulation_messages[i].socket,
                    &(g_delayed_encapsulation_messages[i].message[0]),
                    g_delayed_encapsulation_messages[i].message_size);
                g_delayed_encapsulation_messages[i].socket = -1;
            }
        }
    }
}
