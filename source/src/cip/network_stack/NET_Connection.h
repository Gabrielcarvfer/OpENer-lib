//
// Created by gabriel on 30/11/2016.
//

#ifndef OPENER_NET_CONNECTION_H
#define OPENER_NET_CONNECTION_H

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/sock.h>
#endif

#define INVALID_SOCKET_HANDLE -1
/**
 * @brief NET_Connection abstracts sockets (EthernetIP/TCPIP and DeviceNet/CAN) from CIP Connection
 */
class NET_Connection
{
    public:

        enum { receiver = 0, sender = 1 } socketsBehaviour;
        NET_Connection();
        NET_Connection(struct sockaddr originator_address, struct sockaddr remote_address);
        ~NET_Connection ();

        int InitSocket(int socket_handle_pos, CipUdint family, CipUdint type, CipUdint protocol);
        int SetSocketOpt(int socket_handle_pos, CipUdint type, CipUdint reuse, CipUdint val);
        int BindSocket(int socket_handle_pos, struct sockaddr * address);
        int Listen(int max_num_connections);

        void CloseSocketPlatform(CipUdint socket_handle);

        int GetSocketHandle(int socket_handle_pos);

        int SendData(void * data_ptr, CipUdint size);
        int RecvData (void *data_ptr, CipUdint size);
        int SendDataTo(void * data_ptr, CipUdint size, struct sockaddr * destination);
        int RecvDataFrom (void *data_ptr, CipUdint size, struct sockaddr *source);


    private:
        // socket address for produce
        struct sockaddr *remote_address;

        // the address of the originator that established the connection. needed for
        // scanning if the right packet is arriving
        struct sockaddr *originator_address;

        // socket handles, indexed by kConsuming or kProducing
        int socket[2];
        CipUdint type[2];
        CipUdint reuse[2];
        CipUdint val[2];

        //private functions
        int CheckHandle(int handle);
};


#endif //OPENER_NET_CONNECTION_H
