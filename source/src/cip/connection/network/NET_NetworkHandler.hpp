/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

/** @file generic_networkhandler.c
 *  @author Martin Melik Merkumians
 *  @brief This file includes all platform-independent functions of the network handler to reduce code duplication
 *
 *  @attention This file should only be used for implementing port-specific network handlers and is not intended to grant other parts of the OpENer access to the network layer
 */

#ifndef GENERIC_NETWORKHANDLER_H_
#define GENERIC_NETWORKHANDLER_H_


#include "../../../opener_user_conf.hpp"
#include "../../ciptypes.hpp"
#include "cip/CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "NET_Connection.hpp"

#include "ethIP/NET_EthIP_Includes.h"

#define MAX_NO_OF_TCP_SOCKETS 10

class NET_NetworkHandler
{

public:
        static CipUsint g_ethernet_communication_buffer[PC_OPENER_ETHERNET_BUFFER_SIZE]; /**< communication buffer */

        static int highest_socket_handle; /**< temporary file descriptor for select() */

    /** @brief This variable holds the TCP socket the received to last explicit message.
     * It is needed for opening point to point connection to determine the peer's
     * address.
     */
        static int g_current_active_tcp_socket;

        static struct timeval g_time_value;
        static MilliSeconds g_actual_time;
        static MilliSeconds g_last_time;
        static MilliSeconds g_elapsed_time;

    /** @brief Struct representing the current network status
     *typedef struct
        {
            int tcp_listener; //< TCP listener socket
            int udp_unicast_listener; //< UDP unicast listener socket
            int udp_global_broadcast_listener; //< UDP global network broadcast listener
            MilliSeconds elapsed_time;
        } NetworkStatus;
     */
    enum {tcp_listener, udp_ucast_listener, udp_global_bcast_listener} kNetworkStatus;
    static NET_Connection *netStats[3];


    /** @brief check if on one of the UDP consuming sockets data has been received and if yes handle it correctly
    */
    static void CheckAndHandleConsumingUdpSockets();

    /** @ingroup CIP_CALLBACK_API
    * @brief create a producing or consuming UDP socket
    *
    * @param communication_direction PRODCUER or CONSUMER
    * @param socket_data pointer to the address holding structure
    *     Attention: For producing point-to-point connection the
    *     *pa_pstAddr->sin_addr.s_addr member is set to 0 by OpENer. The network
    *     layer of the application has to set the correct address of the
    *     originator.
    *     Attention: For consuming connection the network layer has to set the
    * pa_pstAddr->sin_addr.s_addr to the correct address of the originator.
    * FIXME add an additional parameter that can be used by the CIP stack to
    * request the originators sockaddr_in data.
    * @return socket identifier on success
    *         -1 on error
    */
    static int CreateUdpSocket(UdpCommuncationDirection communication_direction, struct sockaddr* socket_data);

    /** @brief TODO: FILL IN!
    */
    static CipStatus HandleDataOnTcpSocket(int socket);

    /** @ingroup CIP_CALLBACK_API
     * @brief create a producing or consuming UDP socket
     *
     * @param socket_data pointer to the "send to" address
     * @param socket_handle socket descriptor to send on
     * @param data pointer to the data to send
     * @param data_length length of the data to send
     * @return  EIP_SUCCESS on success
     */
    static CipStatus SendUdpData(struct sockaddr* address, int socket, CipUsint* data, CipUint data_length);

    /** @brief Checks and processes request received via the UDP unicast socket, currently the implementation is port-specific
    */
    static void CheckAndHandleUdpUnicastSocket();

    /** @brief TODO: FILL IN!
    */
    static void CheckAndHandleUdpGlobalBroadcastSocket();

    /** @brief handle any connection request coming in the TCP server socket.
    */
    static void CheckAndHandleTcpListenerSocket();


    //   static NetworkStatus g_network_status; /**< Global variable holding the current network status */

    /** @brief The platform independent part of network handler initialization routine
     *
     *  @return Returns the OpENer status after the initialization routine
     */
        static CipStatus NetworkHandlerInitialize ();
    /*
    void IApp_CloseSocket_udp(int socket_handle);

    void IApp_CloseSocket_tcp(int socket_handle);
    */
    /** @brief Initializes the network handler, shall be implemented by a port-specific networkhandler
     *
     *  @return CipStatus, if initialization failed EipError is returned
     */
        static CipStatus NetworkHandlerProcessOnce ();

        static CipStatus NetworkHandlerFinish ();

    /** @brief check if the given socket is set in the read set
     * @param socket The socket to check
     * @return true if socket is set
     */
        static int CheckSocketSet (int socket);

    /** @brief Returns the socket with the highest id
     * @param socket1 First socket
     * @param socket2 Second socket
     * @param socket3 Third socket
     * @param socket4 Fourth socket
     *
     * @return Highest socket id from the provided sockets
     */
        static int GetMaxSocket (int socket1, int socket2, int socket3, int socket4);

    /** @brief This function shall return the current time in microseconds relative to epoch, and shall be implemented in a port specific networkhandler
     *
     *  @return Current time relative to epoch as MicroSeconds
     */
        static MicroSeconds GetMicroSeconds();

    /** @brief This function shall return the current time in milliseconds relative to epoch, and shall be implemented in a port specific networkhandler
     *
     *  @return Current time relative to epoch as MilliSeconds
     */
        static MilliSeconds GetMilliSeconds();

};
#endif /* GENERIC_NETWORKHANDLER_H_ */