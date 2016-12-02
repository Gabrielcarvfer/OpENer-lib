//
// Created by gabriel on 30/11/2016.
//

#ifndef OPENER_NET_CONNECTION_H
#define OPENER_NET_CONNECTION_H

#ifdef WIN32
    #include <winsock.h>
    #include <windows.h>
#else
    #include <sys/sock.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/select.h>
#endif


#define INVALID_SOCKET_HANDLE -1
/**
 * @brief NET_Connection abstracts sockets (EthernetIP/TCPIP and DeviceNet/CAN) from CIP Connection
 */
class NET_Connection
{
    public:
        //Class stuff
        static void InitSelects();

        //Instance stuff
        enum { receiver = 0, sender = 1 } socketsBehaviour;
        NET_Connection();
        NET_Connection(struct sockaddr originator_address, struct sockaddr remote_address);
        ~NET_Connection ();

        int InitSocket(int socket_handle_pos, CipUdint family, CipUdint type, CipUdint protocol);
        int SetSocketOpt(int socket_handle_pos, CipUdint type, CipUdint reuse, CipUdint val);
        int BindSocket(int socket_handle_pos, struct sockaddr * address);
        int Listen(int max_num_connections);

        void CloseSocket(CipUdint socket_handle);

        int GetSocketHandle(int socket_handle_pos);
        int SetSocketHandle(int socket_handle_pos, int socket_handle);

        int SendData(void * data_ptr, CipUdint size);
        int RecvData (void *data_ptr, CipUdint size);
        int SendDataTo(void * data_ptr, CipUdint size, struct sockaddr * destination);
        int RecvDataFrom (void *data_ptr, CipUdint size, struct sockaddr *source);


    private:
        static fd_set master_socket;
        static fd_set read_socket;
        static std::map <int, NET_Connection*> socket_to_conn_map;

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
