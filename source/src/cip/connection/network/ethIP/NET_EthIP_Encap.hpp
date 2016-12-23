/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_ENCAP_H_
#define OPENER_ENCAP_H_

#define ENCAP_NUMBER_OF_SUPPORTED_DELAYED_ENCAP_MESSAGES 2 /**< According to EIP spec at least 2 delayed message requests should be supported */

#define ENCAP_MAX_DELAYED_ENCAP_MESSAGE_SIZE (ENCAPSULATION_HEADER_LENGTH + 39 + sizeof(OPENER_DEVICE_NAME)) /* currently we only have the size of an encapsulation message */

#include "../../../ciptypes.hpp"
#include "../../../../opENer_user_conf.hpp"
#include "../NET_Encapsulation.hpp"

/** @file encap.h
 * @brief This file contains the public interface of the encapsulation layer
 */

/**  @defgroup ENCAP OpENer Ethernet encapsulation layer
 * The Ethernet encapsulation layer handles provides the abstraction between the Ethernet and the CIP layer.
 */

typedef enum {
    kSessionStatusInvalid = -1,
    kSessionStatusValid = 0
} SessionStatus;

class NET_EthIP_Encap
{
public:
/*** defines ***/

#define ENCAPSULATION_HEADER_LENGTH 24

/** @brief Ethernet/IP standard port */
    static const int kOpENerEthernetPort;

/** @brief definition of status codes in encapsulation protocol
 * All other codes are either legacy codes, or reserved for future use
 *  */
    typedef enum
    {
        kEncapsulationProtocolSuccess = 0x0000,
        kEncapsulationProtocolInvalidCommand = 0x0001,
        kEncapsulationProtocolInsufficientMemory = 0x0002,
        kEncapsulationProtocolIncorrectData = 0x0003,
        kEncapsulationProtocolInvalidSessionHandle = 0x0064,
        kEncapsulationProtocolInvalidLength = 0x0065,
        kEncapsulationProtocolUnsupportedProtocol = 0x0069
    } EncapsulationProtocolErrorCode;


/*** global variables (public) ***/

/*** public functions ***/
/** @ingroup ENCAP
 * @brief Initialize the encapsulation layer.
 */
    static  void EncapsulationInit (void);

/** @ingroup ENCAP
 * @brief Shutdown the encapsulation layer.
 *
 * This means that all open sessions including their sockets are closed.
 */
    static void EncapsulationShutDown (void);

/** @ingroup ENCAP
 * @brief Handle delayed encapsulation message responses
 *
 * Certain encapsulation message requests require a delayed sending of the response
 * message. This functions checks if messages need to be sent and performs the
 * sending.
 */
    static void ManageEncapsulationMessages (MilliSeconds elapsed_time);

/** @ingroup CIP_API
     * @brief Notify the encapsulation layer that an explicit message has been
     * received via TCP.
     *
     * @param socket_handle socket handle from which data is received.
     * @param buffer buffer that contains the received data. This buffer will also
     * contain the response if one is to be sent.
     * @param buffer length of the data in buffer.
     * @param number_of_remaining_bytes return how many bytes of the input are left
     * over after we're done here
     * @return length of reply that need to be sent back
     */
    static int HandleReceivedExplictTcpData (int socket, CipUsint *buffer, unsigned int buffer_length,
                                      int *number_of_remaining_bytes);

/** @ingroup CIP_API
 * @brief Notify the encapsulation layer that an explicit message has been
 * received via UDP.
 *
 * @param socket_handle socket handle from which data is received.
 * @param from_address remote address from which the data is received.
 * @param buffer buffer that contains the received data. This buffer will also
 * contain the response if one is to be sent.
 * @param buffer_length length of the data in buffer.
 * @param number_of_remaining_bytes return how many bytes of the input are left
 * over after we're done here
 * @return length of reply that need to be sent back
 */
    static int HandleReceivedExplictUdpData (int socket, struct sockaddr* from_address, CipUsint* buffer, unsigned int buffer_length, int* number_of_remaining_bytes, int unicast);


private:
    static const int kSupportedProtocolVersion; /**< Supported Encapsulation protocol version */

    static const int kEncapsulationHeaderOptionsFlag; /**< Mask of which options are supported as of the current CIP specs no other option value as 0 should be supported.*/

    static const int kEncapsulationHeaderSessionHandlePosition; /**< the position of the session handle within the encapsulation header*/

    static const int kListIdentityDefaultDelayTime; /**< Default delay time for List Identity response */
    static const int kListIdentityMinimumDelayTime; /**< Minimum delay time for List Identity response */



    static const int kSenderContextSize; /**< size of sender context in encapsulation header*/

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


/* Encapsulation layer data  */

/** @brief Delayed Encapsulation Message structure */
    typedef struct {
        CipDint time_out; /**< time out in milli seconds */
        int socket; /**< associated socket */
        struct sockaddr_in *receiver;
        CipByte message[ENCAP_MAX_DELAYED_ENCAP_MESSAGE_SIZE];
        unsigned int message_size;
    } DelayedEncapsulationMessage;

    static EncapsulationInterfaceInformation g_interface_information;

    static int g_registered_sessions[];

    static DelayedEncapsulationMessage g_delayed_encapsulation_messages[];

/*** private functions ***/
    static void HandleReceivedListServicesCommand(EncapsulationData* receive_data);

    static void HandleReceivedListInterfacesCommand(EncapsulationData* receive_data);

    static void HandleReceivedListIdentityCommandTcp(EncapsulationData* receive_data);

    static void HandleReceivedListIdentityCommandUdp(int socket, struct sockaddr_in* from_address, EncapsulationData* receive_data);

    static void HandleReceivedRegisterSessionCommand(int socket,
                                              EncapsulationData* receive_data);

    static CipStatus HandleReceivedUnregisterSessionCommand(
            EncapsulationData* receive_data);

    static CipStatus HandleReceivedSendUnitDataCommand(EncapsulationData* receive_data);

    static CipStatus HandleReceivedSendRequestResponseDataCommand(
            EncapsulationData* receive_data);

    static int GetFreeSessionIndex(void);

    static CipInt CreateEncapsulationStructure(CipUsint* receive_buffer,
                                        int receive_buffer_length,
                                        EncapsulationData* encapsulation_data);

    static SessionStatus CheckRegisteredSessions(EncapsulationData* receive_data);

    static int EncapsulateData(const EncapsulationData* const send_data);

    static void DetermineDelayTime(CipByte* buffer_start,
                            DelayedEncapsulationMessage* delayed_message_buffer);

    static int EncapsulateListIdentyResponseMessage(CipByte* const communication_buffer);
    static void CloseSession(int socket);


};
#endif /* OPENER_ENCAP_H_ */
