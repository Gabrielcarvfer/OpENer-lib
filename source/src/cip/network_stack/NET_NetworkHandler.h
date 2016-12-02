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


#include <src/opener_user_conf.h>
#include "typedefs.h"
#include "../connection_stack/CIP_Connection.h"
#include "NET_Connection.h"

#define MAX_NO_OF_TCP_SOCKETS 10

class NET_NetworkHandler
{
    private:

        static CipUsint g_ethernet_communication_buffer[PC_OPENER_ETHERNET_BUFFER_SIZE]; /**< communication buffer */

        static fd_set master_socket;
        static fd_set read_socket;

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


    //   static NetworkStatus g_network_status; /**< Global variable holding the current network status */

    /** @brief The platform independent part of network handler initialization routine
     *
     *  @return Returns the OpENer status after the initialization routine
     */
        static CipStatus NetworkHandlerInitialize (void);
    /*
    void IApp_CloseSocket_udp(int socket_handle);

    void IApp_CloseSocket_tcp(int socket_handle);
    */
    /** @brief Initializes the network handler, shall be implemented by a port-specific networkhandler
     *
     *  @return CipStatus, if initialization failed EipError is returned
     */
        static CipStatus NetworkHandlerProcessOnce (void);

        static CipStatus NetworkHandlerFinish (void);

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
        static MicroSeconds GetMicroSeconds(void);

    /** @brief This function shall return the current time in milliseconds relative to epoch, and shall be implemented in a port specific networkhandler
     *
     *  @return Current time relative to epoch as MilliSeconds
     */
        static MilliSeconds GetMilliSeconds(void);
private:
    static void CheckAndHandleConsumingUdpSockets(void);
    static int CreateUdpSocket(UdpCommuncationDirection communication_direction, struct sockaddr* socket_data);
    static CipStatus HandleDataOnTcpSocket(int socket);
    static CipStatus SendUdpData(struct sockaddr* address, int socket, CipUsint* data, CipUint data_length);
    static void CheckAndHandleUdpUnicastSocket(void);
    static void CheckAndHandleUdpGlobalBroadcastSocket(void);
    static void CheckAndHandleTcpListenerSocket(void);
};
#endif /* GENERIC_NETWORKHANDLER_H_ */