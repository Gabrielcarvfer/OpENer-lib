//
// Created by gabriel on 30/11/2016.
//

#include "NET_Connection.h"
NET_Connection::NET_Connection(struct sockaddr_in *originator_address, struct sockaddr_in *remote_address)
{
    originator_address = originator_address;
    remote_address = remote_address;
    socket[0] = -1;
    socket[1] = -1;
}

NET_Connection::~NET_Connection ()
{
    if (socket[0] != -1)
        CloseSocketPlatform (socket[0]);
    if (socket[1] != -1)
        CloseSocketPlatform (socket[1]);
}

int NET_Connection::CloseSocketPlatform(int socket_handle_pos)
{
    if (socket_handle_pos > -1 & socket_handle_pos < 2)
    {
#ifndef WIN32
        shutdown(socket[socket_handle_pos], SHUT_RDWR);
#endif
        closesocket(socket[socket_handle_pos]);
    }
    //Invalid handle
    return -1;
}

int NET_Connection::GetSocket(int socket_handle_pos)
{
    if (socket_handle_pos > -1 & socket_handle_pos < 2)
    {
        return socket[pos];
    }
    //Invalid handle
    return -1;
}

int NET_Connection::InitSocket(int socket_handle_pos, CipUdint family, CipUdint type, CipUdint protocol)
{
    if (socket_handle_pos > -1 & socket_handle_pos < 2)
    {
        //Examples of parameters
        // family: AF_INET,PF_CAN
        // type: SOCK_STREAM,SOCK_RAW
        // protocol: IPPROTO_TCP,CAN_RAW
        return socket[socket_handle_pos] = socket(family, type, protocol);
    }
    //Invalid handle
    return -1;
}

int NET_Connection::SetSocketOpt(int socket_handle_pos, CipUdint type, CipUdint reuse, CipUdint val)
{
    if (socket_handle_pos > -1 & socket_handle_pos < 2)
    {
        type[socket_handle_pos] = type;
        reuse[socket_handle_pos] = type;
        return setsockopt (socket[socked_handle_pos], type, reuse, (char *) &val, sizeof (CipUdint));
    }
    //Invalid handle
    return -1;
}