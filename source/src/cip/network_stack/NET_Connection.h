//
// Created by gabriel on 30/11/2016.
//

#ifndef OPENER_NET_CONNECTION_H
#define OPENER_NET_CONNECTION_H

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/sock.h>
#endif

/**
 * @brief NET_Connection abstracts sockets (EthernetIP/TCPIP and DeviceNet/CAN) from CIP Connection
 */
class NET_Connection
{
    public:
        NET_Connection(struct sockaddr_in originator_address, struct sockaddr_in remote_address);
        ~NET_Connection ();
        int GetSocket(int socket_handle_pos);
        void CloseSocketPlatform(CipUdint socket_handle);
        int SetSocketOpt(int socket_handle_pos, CipUdint type, CipUdint reuse, CipUdint val);

        enum { receiver = 0, sender = 1 }socketBehaviour;

    private:
        // socket address for produce
        struct sockaddr_in *remote_address;

        // the address of the originator that established the connection. needed for
        // scanning if the right packet is arriving
        struct sockaddr_in *originator_address;

        // socket handles, indexed by kConsuming or kProducing
        int socket[2];
        CipUdint type[2];
        CipUdint reuse[2];
        CipUdint val[2];
};


#endif //OPENER_NET_CONNECTION_H
