//
// Created by gabriel on 30/11/2016.
//

#include "NET_Connection.h"

NET_Connection::NET_Connection()
{
    socket[0] = INVALID_SOCKET_HANDLE;
    socket[1] = INVALID_SOCKET_HANDLE;
}
NET_Connection::NET_Connection(struct sockaddr_in *originator_address, struct sockaddr_in *remote_address)
{
    originator_address = originator_address;
    remote_address = remote_address;

    socket[0] = INVALID_SOCKET_HANDLE;
    socket[1] = INVALID_SOCKET_HANDLE;
}

NET_Connection::~NET_Connection ()
{
    if (socket[0] != INVALID_SOCKET_HANDLE)
        CloseSocketPlatform (socket[0]);
    if (socket[1] != INVALID_SOCKET_HANDLE)
        CloseSocketPlatform (socket[1]);

    delete remote_address;
    delete originator_address;
}

int NET_Connection::InitSocket(int socket_handle_pos, CipUdint family, CipUdint type, CipUdint protocol)
{
    if (CheckHandle(socket_handle_pos))
    {
        //Examples of parameters
        // family: AF_INET,PF_CAN
        // type: SOCK_STREAM,SOCK_RAW
        // protocol: IPPROTO_TCP,CAN_RAW
        return socket[socket_handle_pos] = socket(family, type, protocol);
    }
    return INVALID_SOCKET_HANDLE;
}

int NET_Connection::SetSocketOpt(int socket_handle_pos, CipUdint type, CipUdint reuse, CipUdint val)
{
    if (CheckHandle(socket_handle_pos))
    {
        type[socket_handle_pos] = type;
        reuse[socket_handle_pos] = type;
        return setsockopt (socket[socked_handle_pos], type, reuse, (char *) &val, sizeof (CipUdint));
    }
    return INVALID_SOCKET_HANDLE;
}

int NET_Connection::BindSocket(int socket_handle_pos, struct sockaddr_in * address)
{
    if (CheckHandle (socket_handle_pos))
    {
        //todo: check if the receiver is the originator address or remote_address
        if (socket_handle_pos == receiver)
        {
            originator_address = address;
        }
        else
        {
            remote_address = address;
        }
        return bind(socket[socket_handle_pos], address, sizeof(struct sockaddr));
    }
    return INVALID_SOCKET_HANDLE;
}

int NET_Connection::CloseSocketPlatform(int socket_handle_pos)
{
    if (CheckHandle(socket_handle_pos))
    {
#ifndef WIN32
        shutdown(socket[socket_handle_pos], SHUT_RDWR);
#endif
        closesocket(socket[socket_handle_pos]);
    }
    return INVALID_SOCKET_HANDLE;
}

int NET_Connection::GetSocketHandle(int socket_handle_pos)
{
    if (CheckHandle(socket_handle_pos))
    {
        return socket[pos];
    }
    return INVALID_SOCKET_HANDLE;
}

int NET_Connection::SendData(void * data_ptr, CipUdint size)
{
    return write(socket[sender], data_ptr, size);
}

int NET_Connection::ReceiveData(void * data_ptr, CipUdint size)
{
    return read(socket[receiver], data_ptr, size);
}

int NET_Connection::CheckHandle(int handle)
{
    if (handle > -1 & handle < 2)
        return 1;
    else
        return 0;
}