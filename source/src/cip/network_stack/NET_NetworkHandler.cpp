/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

/** @file generic_networkhandler.c
 *  @author Martin Melik Merkumians
 *  @brief This file includes all functions of the network handler to reduce code duplication
 *
 */

#include <src/utils/endianconv.h>
#include "NET_NetworkHandler.h"

#include "trace.h"

#include "src/cip/network_stack/ethernetip_net/tcpip_link/ciptcpipinterface.h"

#ifdef WIN32
    typedef unsigned long socklen_t;
#endif


CipStatus NET_NetworkHandler::NetworkHandlerInitialize()
{
    #ifdef WIN32
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        WSAStartup(wVersionRequested, &wsaData);
    #endif

    NET_Connection::InitSelects();

    // create a new TCP socket
    netStats[tcp_listener] = new NET_Connection();
    if (netStats[tcp_listener]->InitSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == -1)
    {
        OPENER_TRACE_ERR("error allocating socket stream listener, %d\n", errno);
        return kCipStatusError;
    }
    //Represents true for used set socket options
    CipUdint set_socket_option = 1;

    // Activates address reuse
    if (netStats[tcp_listener]->SetSocketOpt(SOL_SOCKET, SO_REUSEADDR, set_socket_option) == -1)
    {
        OPENER_TRACE_ERR("error setting socket option SO_REUSEADDR on tcp_listener\n");
        return kCipStatusError;
    }

    // create a new UDP socket for broadcast
    netStats[udp_global_bcast_listener] = new NET_Connection();
    if (netStats[udp_global_bcast_listener]->InitSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) == -1)
    {
        OPENER_TRACE_ERR("error allocating UDP global broadcast listener socket, %d\n", errno);
        return kCipStatusError;
    }

    // create a new UDP socket for unicast
    netStats[udp_ucast_listener] = new NET_Connection();
    if (netStats[udp_ucast_listener]->InitSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) == -1)
    {
        OPENER_TRACE_ERR("error allocating UDP unicast listener socket, %d\n", errno);
        return kCipStatusError;
    }

    // Activates address reuse
    if (netStats[udp_global_bcast_listener]->SetSocketOpt(SOL_SOCKET, SO_REUSEADDR, set_socket_option) == -1)
    {
        OPENER_TRACE_ERR("error setting socket option SO_REUSEADDR on udp_broadcast_listener\n");
        return kCipStatusError;
    }

    // Activates address reuse
    if ( netStats[udp_ucast_listener]->SetSocketOpt(SOL_SOCKET, SO_REUSEADDR, set_socket_option) == -1)
    {
        OPENER_TRACE_ERR("error setting socket option SO_REUSEADDR on udp_ucast_listener\n");
        return kCipStatusError;
    }

    struct sockaddr_in *my_address;
    my_address = new struct sockaddr_in();
    my_address->sin_family = AF_INET;
    my_address->sin_port = htons(kOpenerEthernetPort);
    my_address->sin_addr.s_addr = interface_configuration_.ip_address;

    // bind the new socket to port 0xAF12 (CIP)
    if (netStats[tcp_listener]->BindSocket((struct sockaddr *) my_address) == -1)
    {
        OPENER_TRACE_ERR("error with TCP bind: %s\n", strerror(errno));
        return kCipStatusError;
    }

    if (netStats[udp_ucast_listener]->BindSocket((struct sockaddr *) my_address) == -1)
    {
        OPENER_TRACE_ERR("error with UDP unicast bind: %s\n", strerror(errno));
        return kCipStatusError;
    }

    struct sockaddr_in *global_broadcast_address;
    global_broadcast_address = new struct sockaddr_in();
    global_broadcast_address->sin_family = AF_INET;
    global_broadcast_address->sin_port = htons(kOpenerEthernetPort);
    global_broadcast_address->sin_addr.s_addr = htonl(INADDR_ANY);

    // enable the UDP socket to receive broadcast messages
    if ( netStats[udp_global_bcast_listener]->SetSocketOpt (SOL_SOCKET, SO_BROADCAST, set_socket_option) < 0)
    {
        OPENER_TRACE_ERR("error with setting broadcast receive for UDP socket: %s\n", strerror(errno));
        return kCipStatusError;
    }

    if (netStats[udp_global_bcast_listener]->BindSocket ((struct sockaddr*)global_broadcast_address) == -1)
    {
        OPENER_TRACE_ERR("error with global broadcast UDP bind: %s\n", strerror(errno));
        return kCipStatusError;
    }

//TODO:-------------------------------
//TODO: finish fixing newer stuff
//TODO:-------------------------------
    // switch socket in listen mode
    if (netStats[tcp_listener]->Listen(MAX_NO_OF_TCP_SOCKETS) == -1)
    {
        OPENER_TRACE_ERR("networkhandler: error with listen: %s\n", strerror(errno));
        return kCipStatusError;
    }

    // add the listener socket to the master set
    NET_Connection::SelectSet(netStats[tcp_listener]->GetSocketHandle(), NET_Connection::kMasterSet);
    NET_Connection::SelectSet(netStats[udp_ucast_listener]->GetSocketHandle(), NET_Connection::kMasterSet);
    NET_Connection::SelectSet(netStats[udp_global_bcast_listener]->GetSocketHandle(), NET_Connection::kMasterSet);

    // keep track of the biggest file descriptor
    highest_socket_handle = GetMaxSocket(
            netStats[tcp_listener]->GetSocketHandle(),
            netStats[udp_ucast_listener]->GetSocketHandle(),
            netStats[udp_global_bcast_listener]->GetSocketHandle(),
            0);

    g_last_time = GetMilliSeconds(); // initialize time keeping
    g_elapsed_time = 0;

    return kCipStatusOk;
}


int NET_NetworkHandler::CheckSocketSet(int socket)
{
    if (NET_Connection::SelectIsSet(socket, NET_Connection::kReadSet) != -1)
    {
        if (NET_Connection::SelectIsSet(socket, NET_Connection::kMasterSet) != -1)
        {
            return 1;
        }
        else
        {
            OPENER_TRACE_INFO("socket: %d closed with pending message\n", socket);
        }
        NET_Connection::SelectRemove (socket, NET_Connection::kReadSet);
        // remove it from the read set so that later checks will not find it
    }
    return 0;
}

void NET_NetworkHandler::CheckAndHandleTcpListenerSocket(void)
{
    //TODO: create new connection
    int new_socket;
    // see if this is a connection request to the TCP listener
    if (CheckSocketSet(netStats[tcp_listener]->GetSocketHandle()))
    {
        OPENER_TRACE_INFO("networkhandler: new TCP connection\n");

        new_socket = accept(netStats[tcp_listener]->GetSocketHandle(), NULL, NULL);
        if (new_socket == -1)
        {
            OPENER_TRACE_ERR("networkhandler: error on accept: %s\n", strerror(errno));
            return;
        }

        NET_Connection::SelectSet(new_socket, NET_Connection::kMasterSet);
        // add newfd to master set
        if (new_socket > highest_socket_handle)
        {
            highest_socket_handle = new_socket;
        }

        OPENER_TRACE_STATE("networkhandler: opened new TCP connection on fd %d\n", new_socket);
    }
}

CipStatus NET_NetworkHandler::NetworkHandlerProcessOnce(void)
{
    NET_Connection::SelectCopy();

    g_time_value.tv_sec = 0;
    g_time_value.tv_usec = (g_elapsed_time < kOpenerTimerTickInMilliSeconds ? kOpenerTimerTickInMilliSeconds - g_elapsed_time : 0) * 1000; // 10 ms

    int ready_socket = NET_Connection::SelectSelect(highest_socket_handle + 1, NET_Connection::kReadSet, &g_time_value);

    if (ready_socket == kEipInvalidSocket)
    {
        if (EINTR == errno) // we have somehow been interrupted. The default behavior is to go back into the select loop.
        {
            return kCipStatusOk;
        }
        else
        {
            OPENER_TRACE_ERR("networkhandler: error with select: %s\n", strerror(errno));
            return kCipStatusError;
        }
    }

    if (ready_socket > 0)
    {

        CheckAndHandleTcpListenerSocket();
        CheckAndHandleUdpUnicastSocket();
        CheckAndHandleUdpGlobalBroadcastSocket();
        CheckAndHandleConsumingUdpSockets();

        for (int socket = 0; socket <= highest_socket_handle; socket++)
        {
            if (CheckSocketSet(socket))
            {
                // if it is still checked it is a TCP receive
                if (kCipStatusError == HandleDataOnTcpSocket(socket)) // if error
                {
                    //todo: move to CIP_Connection
                    // CloseSocket(socket);
                    // CloseSession(socket); // clean up session and close the socket
                }
            }
        }
    }

    g_actual_time = GetMilliSeconds();
    g_elapsed_time += g_actual_time - g_last_time;
    g_last_time = g_actual_time;

    // check if we had been not able to update the connection manager for several OPENER_TIMER_TICK.
    // This should compensate the jitter of the windows timer
    if (g_elapsed_time >= kOpenerTimerTickInMilliSeconds)
    {
        /* call manage_connections() in connection manager every OPENER_TIMER_TICK ms */
        CIP_Connection::ManageConnections(g_elapsed_time);
        g_elapsed_time = 0;
    }
    return kCipStatusOk;
}

//todo:move to CIP_Connection
/*void NET_NetworkHandler::IApp_CloseSocket_udp(int socket_handle)
{
    CloseSocket(socket_handle);
}

void NET_NetworkHandler::IApp_CloseSocket_tcp(int socket_handle)
{
    CloseSocket(socket_handle);
}*/
CipStatus NET_NetworkHandler::NetworkHandlerFinish(void)
{
    netStats[tcp_listener]->CloseSocket ();
    netStats[udp_ucast_listener]->CloseSocket ();
    netStats[udp_global_bcast_listener]->CloseSocket ();
    return kCipStatusOk;
}

void NET_NetworkHandler::CheckAndHandleUdpGlobalBroadcastSocket(void)
{

    struct sockaddr_in from_address;
    socklen_t from_address_length;

    // see if this is an unsolicited inbound UDP message
    if (CheckSocketSet(netStats[udp_global_bcast_listener]->GetSocketHandle()))
    {

        from_address_length = sizeof(from_address);

        OPENER_TRACE_STATE("networkhandler: unsolicited UDP message on EIP global broadcast socket\n");

        // Handle UDP broadcast messages
        int received_size = netStats[udp_global_bcast_listener]->RecvDataFrom (&g_ethernet_communication_buffer,
            PC_OPENER_ETHERNET_BUFFER_SIZE, (struct sockaddr*)&from_address);

        if (received_size <= 0)
        {
            // got error
            OPENER_TRACE_ERR("networkhandler: error on recvfrom UDP global broadcast port: %s\n", strerror(errno));
            return;
        }

        OPENER_TRACE_INFO("Data received on global broadcast UDP:\n");

        CipUsint* receive_buffer = &g_ethernet_communication_buffer[0];
        int remaining_bytes = 0;
        do
        {
            int reply_length = HandleReceivedExplictUdpData(netStats[udp_global_bcast_listener]->GetSocketHandle(),
                                                            (struct sockaddr*)&from_address, receive_buffer, received_size, &remaining_bytes, false);

            receive_buffer += received_size - remaining_bytes;
            received_size = remaining_bytes;

            if (reply_length > 0)
            {
                OPENER_TRACE_INFO("reply sent:\n");

                // if the active socket matches a registered UDP callback, handle a UDP packet
                if (netStats[udp_global_bcast_listener]->SendDataTo(&g_ethernet_communication_buffer, reply_length, (struct sockaddr*)&from_address) != reply_length)
                {
                    OPENER_TRACE_INFO("networkhandler: UDP response was not fully sent\n");
                }
            }
        } while (remaining_bytes > 0);
    }
}

void NET_NetworkHandler::CheckAndHandleUdpUnicastSocket(void)
{

    struct sockaddr_in from_address;
    socklen_t from_address_length;

    /* see if this is an unsolicited inbound UDP message */
    if (CheckSocketSet(netStats[udp_ucast_listener]->GetSocketHandle ()))
    {

        from_address_length = sizeof(from_address);

        OPENER_TRACE_STATE("networkhandler: unsolicited UDP message on EIP unicast socket\n");

        // Handle UDP broadcast messages
        int recv_size = netStats[udp_ucast_listener]->RecvDataFrom (&g_ethernet_communication_buffer, PC_OPENER_ETHERNET_BUFFER_SIZE, (struct sockaddr *) &from_address);


        if (recv_size <= 0)
        {
            // got error
            OPENER_TRACE_ERR("networkhandler: error on recvfrom UDP unicast port: %s\n", strerror(errno));
            return;
        }

        OPENER_TRACE_INFO("Data received on UDP unicast:\n");

        CipUsint* receive_buffer = &g_ethernet_communication_buffer[0];
        int remaining_bytes = 0;
        do
        {
            int reply_length = HandleReceivedExplictUdpData(netStats[udp_ucast_listener]->GetSocketHandle (), (struct sockaddr*)&from_address, receive_buffer, recv_size, &remaining_bytes, true);

            receive_buffer += recv_size - remaining_bytes;
            recv_size = remaining_bytes;

            if (reply_length > 0)
            {
                OPENER_TRACE_INFO("reply sent:\n");

                // if the active socket matches a registered UDP callback, handle a UDP packet
                if (netStats[udp_ucast_listener]->SendDataTo(&g_ethernet_communication_buffer, reply_length,  (struct sockaddr*)&from_address) != reply_length)
                {
                    OPENER_TRACE_INFO("networkhandler: UDP unicast response was not fully sent\n");
                }
            }
        } while (remaining_bytes > 0);
    }
}

CipStatus NET_NetworkHandler::SendUdpData(struct sockaddr* address, int socket, CipUsint* data, CipUint data_length)
{

    int sent_length = sendto(socket, (char*)data, data_length, 0, address, sizeof(struct sockaddr));

    if (sent_length < 0)
    {
        OPENER_TRACE_ERR("networkhandler: error with sendto in sendUDPData: %s\n", strerror(errno));
        return kCipStatusError;
    }

    if (sent_length != data_length)
    {
        OPENER_TRACE_WARN("data length sent_length mismatch; probably not all data was sent in SendUdpData, sent %d of %d\n", sent_length, data_length);
        return kCipStatusError;
    }

    return kCipStatusOk;
}

CipStatus NET_NetworkHandler::HandleDataOnTcpSocket(int socket)
{
    int remaining_bytes = 0;
    long data_sent = PC_OPENER_ETHERNET_BUFFER_SIZE;

    /* We will handle just one EIP packet here the rest is done by the select
   * method which will inform us if more data is available in the socket
   because of the current implementation of the main loop this may not be
   the fastest way and a loop here with a non blocking socket would better
   fit*/

    /*Check how many data is here -- read the first four bytes from the connection */
    /*TODO we may have to set the socket to a non blocking socket */
    long number_of_read_bytes = recv(socket, (char*)g_ethernet_communication_buffer, 4, 0);

    if (number_of_read_bytes == 0)
    {
        OPENER_TRACE_ERR("networkhandler: connection closed by client: %s\n", strerror(errno));
        return kCipStatusError;
    }
    if (number_of_read_bytes < 0)
    {
        OPENER_TRACE_ERR("networkhandler: error on recv: %s\n", strerror(errno));
        return kCipStatusError;
    }

    // at this place EIP stores the data length
    CipUsint* read_buffer = &g_ethernet_communication_buffer[2];
    // -4 is for the 4 bytes we have already read
    size_t data_size = GetIntFromMessage(&read_buffer) + ENCAPSULATION_HEADER_LENGTH - 4;
    // (NOTE this advances the buffer pointer)

    // TODO can this be handled in a better way?
    if ((PC_OPENER_ETHERNET_BUFFER_SIZE - 4) < data_size)
    {
        OPENER_TRACE_ERR("too large packet received will be ignored, will drop the data\n");

        // Currently we will drop the whole packet
        do
        {
            number_of_read_bytes = recv(socket, (char*)&g_ethernet_communication_buffer[0], data_sent, 0);

            if (number_of_read_bytes == 0) /* got error or connection closed by client */
            {
                OPENER_TRACE_ERR("networkhandler: connection closed by client: %s\n", strerror(errno));
                return kCipStatusError;
            }
            if (number_of_read_bytes < 0)
            {
                OPENER_TRACE_ERR("networkhandler: error on recv: %s\n", strerror(errno));
                return kCipStatusError;
            }
            data_size -= number_of_read_bytes;
            if ((data_size < PC_OPENER_ETHERNET_BUFFER_SIZE) && (data_size != 0))
            {
                data_sent = data_size;
            }
        } while (0 != data_size); /* TODO: fragile end statement */
        return kCipStatusOk;
    }

    number_of_read_bytes = recv(socket, (char*) &g_ethernet_communication_buffer[4], data_size, 0);

    if (number_of_read_bytes == 0) /* got error or connection closed by client */
    {
        OPENER_TRACE_ERR("networkhandler: connection closed by client: %s\n", strerror(errno));
        return kCipStatusError;
    }
    if (number_of_read_bytes < 0)
    {
        OPENER_TRACE_ERR("networkhandler: error on recv: %s\n", strerror(errno));
        return kCipStatusError;
    }

    if ((unsigned)number_of_read_bytes == data_size)
    {
        /*we got the right amount of data */
        data_size += 4;
        /*TODO handle partial packets*/
        OPENER_TRACE_INFO("Data received on tcp:\n");

        g_current_active_tcp_socket = socket;

        number_of_read_bytes = HandleReceivedExplictTcpData(socket, g_ethernet_communication_buffer, data_size, &remaining_bytes);

        g_current_active_tcp_socket = -1;

        if (remaining_bytes != 0)
        {
            OPENER_TRACE_WARN("Warning: received packet was to long: %d Bytes left!\n", remaining_bytes);
        }

        if (number_of_read_bytes > 0)
        {
            OPENER_TRACE_INFO("reply sent:\n");

            data_sent = send(socket, (char*)&g_ethernet_communication_buffer[0], number_of_read_bytes, 0);
            if (data_sent != number_of_read_bytes)
            {
                OPENER_TRACE_WARN("TCP response was not fully sent\n");
            }
        }

        return kCipStatusOk;
    }
    else
    {
        // we got a fragmented packet currently we cannot handle this will
        // for this we would need a network buffer per TCP socket
        //
        // However with typical packet sizes of EIP this should't be a big issue.
        //
        //TODO handle fragmented packets */
    }
    return kCipStatusError;
}

/** @brief create a new UDP socket for the connection manager
 *
 * @param communciation_direction Consuming or producing port
 * @param socket_data Data for socket creation
 *
 * @return the socket handle if successful, else -1 */
int NET_NetworkHandler::CreateUdpSocket(UdpCommuncationDirection communication_direction, struct sockaddr* socket_data)
{
    struct sockaddr_in peer_address;
    struct sockaddr_in * socket_data_in = (struct sockaddr_in*)socket_data;
    int new_socket;

    socklen_t peer_address_length;

    peer_address_length = sizeof(struct sockaddr_in);
    // create a new UDP socket
    if ((new_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        OPENER_TRACE_ERR("networkhandler: cannot create UDP socket: %s\n", strerror(errno));
        return kEipInvalidSocket;
    }

    OPENER_TRACE_INFO("networkhandler: UDP socket %d\n", new_socket);

    /* check if it is sending or receiving */
    if (communication_direction == kUdpCommuncationDirectionConsuming)
    {
        int option_value = 1;
        if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&option_value, sizeof(option_value)) == -1) {
            OPENER_TRACE_ERR("error setting socket option SO_REUSEADDR on consuming udp socket\n");
            return kCipStatusError;
        }

        /* bind is only for consuming necessary */
        if ((bind(new_socket, socket_data, sizeof(struct sockaddr))) == -1)
        {
            OPENER_TRACE_ERR("error on bind udp: %s\n", strerror(errno));
            return kEipInvalidSocket;
        }

        OPENER_TRACE_INFO("networkhandler: bind UDP socket %d\n", new_socket);
    }
    else
    {
        // we have a producing udp socket
        if (socket_data_in->sin_addr.s_addr == g_multicast_configuration.starting_multicast_address)
        {
            if (1 != g_time_to_live_value)
            {
                // we need to set a TTL value for the socket
                if (setsockopt(new_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&g_time_to_live_value, sizeof(g_time_to_live_value)) < 0)
                {
                    OPENER_TRACE_ERR("networkhandler: could not set the TTL to: %d, error: %s\n", g_time_to_live_value, strerror(errno));
                    return kEipInvalidSocket;
                }
            }
        }
    }

    if ((communication_direction == kUdpCommuncationDirectionConsuming) || (0 == socket_data_in->sin_addr.s_addr))
    {
        /* we have a peer to peer producer or a consuming connection*/
        if (getpeername(g_current_active_tcp_socket, (struct sockaddr*)&peer_address, (int*) &peer_address_length) < 0)
        {
            OPENER_TRACE_ERR("networkhandler: could not get peername: %s\n", strerror(errno));
            return kEipInvalidSocket;
        }
        /* store the originators address */
        socket_data_in->sin_addr.s_addr = peer_address.sin_addr.s_addr;
    }

    // add new socket to the master list
    NET_Connection::SelectSet(new_socket, NET_Connection::kMasterSet);
    if (new_socket > highest_socket_handle)
    {
        highest_socket_handle = new_socket;
    }
    return new_socket;
}

void NET_NetworkHandler::CheckAndHandleConsumingUdpSockets(void)
{
    struct sockaddr_in from_address;
    socklen_t from_address_length;

    CIP_Connection* connection_object_iterator;
    CIP_Connection* current_connection_object = NULL;

    // see a message on one of the registered UDP sockets has been received
    for (int i = 0; i < CIP_Connection::active_connections_set.size(); i++)
    {

        // do this at the beginning as the close function may can make the entry invalid
        connection_object_iterator = CIP_Connection::active_connections_set[i];

        if ((-1 != current_connection_object->conn->GetSocketHandle(/*todo:kUdpCommuncationDirectionConsuming*/) && (CheckSocketSet( current_connection_object->conn->GetSocketHandle (/*todo:kUdpCommuncationDirectionConsuming*/)))))
        {
            from_address_length = sizeof(from_address);
            int received_size = recvfrom(current_connection_object->conn->GetSocketHandle(/*kUdpCommuncationDirectionConsuming*/), (char*)g_ethernet_communication_buffer, PC_OPENER_ETHERNET_BUFFER_SIZE, 0, (struct sockaddr*)&from_address, (int*)&from_address_length);
            if (0 == received_size) 
            {
                OPENER_TRACE_STATE("connection closed by client\n");
                current_connection_object->CloseConnection (current_connection_object);
                continue;
            }

            if (0 > received_size) 
            {
                OPENER_TRACE_ERR("networkhandler: error on recv: %s\n", strerror(errno));
                current_connection_object->CloseConnection (current_connection_object);
                continue;
            }

            current_connection_object->HandleReceivedConnectedData(g_ethernet_communication_buffer, received_size, &from_address);
        }
    }
}



int NET_NetworkHandler::GetMaxSocket(int socket1, int socket2, int socket3, int socket4)
{
    if ((socket1 > socket2) & (socket1 > socket3) & (socket1 > socket4))
        return socket1;

    if ((socket2 > socket1) & (socket2 > socket3) & (socket2 > socket4))
        return socket2;

    if ((socket3 > socket1) & (socket3 > socket2) & (socket3 > socket4))
        return socket3;

    return socket4;
}

MicroSeconds NET_NetworkHandler::GetMicroSeconds()
{
    LARGE_INTEGER performance_counter;
    LARGE_INTEGER performance_frequency;

    QueryPerformanceCounter(&performance_counter);
    QueryPerformanceFrequency(&performance_frequency);

    return (MicroSeconds)(performance_counter.QuadPart * 1000000LL / performance_frequency.QuadPart);
}

MilliSeconds NET_NetworkHandler::GetMilliSeconds(void)
{
    return (MilliSeconds)(GetMicroSeconds() / 1000ULL);
}