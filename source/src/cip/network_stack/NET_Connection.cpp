//
// Created by gabriel on 30/11/2016.
//

#include "NET_Connection.h"

void NET_Connection::InitSelects()
{
    // clear the master an temp sets
    FD_ZERO(&master_socket);
    FD_ZERO(&read_socket);
}
void NET_Connection::SelectCopy()
{
    read_socket = master_socket;
}
int NET_Connection::SelectSet(int socket_handle, int select_set_option)
{
    if(CheckSelectSet(select_set_option) & CheckSocketHandle(socket_handle))
    {
        FD_SET(socket_handle, select_set[select_set_option]);
        return 1;
    }
    return INVALID_INPUTS;
}

int NET_Connection::SelectIsSet(int socket_handle, int select_set_option)
{
    if(CheckSelectSet(select_set_option) & CheckSocketHandle(socket_handle))
    {
        return FD_ISSET(socket_handle, select_set[select_set_option]);
    }
    return INVALID_INPUTS;
}

int NET_Connection::SelectSelect(int socket_handle, int select_set_option, struct timeval * time)
{
    if(CheckSelectSet(select_set_option) & CheckSocketHandle(socket_handle))
    {
        return select(socket_handle, select_set[select_set_option], 0, 0, time);
    }
    return INVALID_INPUTS;
}

int NET_Connection::SelectRemove(int socket_handle, int select_set_option)
{
    if(CheckSelectSet(select_set_option))
    {
        FD_CLR(socket_handle, select_set[select_set_option]);
        return 0;
    }
    return INVALID_INPUTS;
}


NET_Connection::NET_Connection()
{
    socket[0] = INVALID_SOCKET_HANDLE;
    socket[1] = INVALID_SOCKET_HANDLE;
}
NET_Connection::NET_Connection(struct sockaddr *originator_address, struct sockaddr *remote_address)
{
    originator_address = originator_address;
    remote_address = remote_address;

    socket = INVALID_SOCKET_HANDLE;
}

NET_Connection::~NET_Connection ()
{
    if (socket != INVALID_SOCKET_HANDLE)
        CloseSocketPlatform (socket);

    delete remote_address;
    delete originator_address;
}

int NET_Connection::InitSocket(CipUdint family, CipUdint type, CipUdint protocol)
{

        int socket;
        //Examples of parameters
        // family: AF_INET,PF_CAN
        // type: SOCK_STREAM,SOCK_RAW
        // protocol: IPPROTO_TCP,CAN_RAW
        socket= socket(family, type, protocol);
        socket_to_conn_map.emplace (socket, this);
        return socket;
}

int NET_Connection::SetSocketOpt(CipUdint type, CipUdint reuse, CipUdint val)
{
        type = type;
        reuse = reuse;
        val = val;
        return setsockopt (socket, type, reuse, (char *) &val, sizeof (CipUdint));
}

int NET_Connection::BindSocket(int address_option, struct sockaddr_in * address)
{
    switch(address_option)
    {
        case kOriginatorAddress:
            originator_address = address;
            break;
        case kRemoteAddress:
            remote_address = address;
            break;
        default:
            return INVALID_INPUTS;
    }
    return bind(socket[socket_handle_pos], address, sizeof(struct sockaddr));
}

int NET_Connection::Listen(int max_num_connections)
{
    return list(socket[receiver], max_num_connections);
}

int NET_Connection::CloseSocket()
{
        OPENER_TRACE_INFO("networkhandler: closing socket %d\n", socket[socket_handle_pos]);
        if (kCipInvalidSocket != socket)
        {
            //Check if socket is still registered
            if (socket_to_conn_map.find(socket) != socket_to_conn_map::end())
            {
                FD_CLR(socket, &master_socket);
#ifndef WIN32
                shutdown(socket, SHUT_RDWR);
#endif
                closesocket (socket);


                socket_to_conn_map.erase (socket);
            }
            //Set socket val to -1
            socket = -1;
        }
        return 0;
}

int NET_Connection::GetSocketHandle(int socket_handle_pos)
{
    return socket;
}

int NET_Connection::SetSocketHandle(int socket_handle)
{
    return socket = socket_handle;
}

int NET_Connection::SendData(void * data_ptr, CipUdint size)
{
    return send(socket, data_ptr, size);
}

int NET_Connection::RecvData (void *data_ptr, CipUdint size)
{
    return recv(socket, data_ptr, size, 0);
}

int NET_Connection::SendDataTo(void * data_ptr, CipUdint size, struct sockaddr * destination)
{
    return sendto(socket, data_ptr, size, 0, destination, sizeof(sockaddr*));
}

int NET_Connection::RecvDataFrom (void *data_ptr, CipUdint size, struct sockaddr *source)
{
    return recvfrom(socket, data_ptr, size, 0, source, sizeof(sockaddr*));
}
