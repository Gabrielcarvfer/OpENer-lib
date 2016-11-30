//
// Created by gabriel on 30/11/2016.
//

#include "NET_Connection.h"
NET_Connection::NET_Connection(struct sockaddr_in originator_address, struct sockaddr_in remote_address)
{

}

NET_Connection::~NET_Connection ()
{
    if (socket[0] != -1)
        CloseSocketPlatform (socket[0]);
    if (socket[1] != -1)
        CloseSocketPlatform (socket[1]);
}

void NET_Connection::CloseSocketPlatform(int socket_handle)
{
#ifndef WIN32
    shutdown(socket_handle, SHUT_RDWR);
#endif
    closesocket(socket_handle);
}

int NET_Connection::GetSocket(int socket_handle_pos)
{
    if (pos > -1 & pos < 2)
    {
        return socket[pos];
    }
    else
    {
        return -1;
    }
}