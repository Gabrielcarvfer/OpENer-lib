//
// Created by Gabriel Ferreira (@gabrielcarvfer)
//

#ifndef OPENER_NET_CONNECTION_H
#define OPENER_NET_CONNECTION_H

#include <map>
#include <ciptypes.hpp>
#include <ethIP/NET_EthIP_Includes.hpp>

#define INVALID_SOCKET_HANDLE -1
#define INVALID_INPUTS -1
/**
 * @brief NET_Connection abstracts sockets (EthernetIP/TCPIP and DeviceNet/CAN) from CIP Connection
 */

class NET_Connection
{
    public:
        //Class stuff
        typedef enum { kMasterSet, kReadSet } SelectSets;
        static void InitSelects();
        static void SelectCopy();
        static int SelectSet    (int socket_handle, int select_set_option);
        static int SelectIsSet  (int socket_handle, int select_set_option);
        static int SelectSelect (int socket_handle, int select_set_option, struct timeval * time);
        static int SelectRemove (int socket_handle, int select_set_option);

        //Instance stuff
        typedef enum
		{
			kOriginatorAddress, kRemoteAddress
		} AddressOptions;

        NET_Connection(
		       struct sockaddr *originator_address = nullptr,
		       struct sockaddr *remote_address = nullptr,
		       CipUdint type = 0,
		       CipUdint reuse = 0,
		       CipUdint direction = 0,
		       CipUdint val = 0,
		       int sock = -1
		       )
        {
		   this->originator_address = originator_address;
		   this->remote_address = remote_address;
		   this->type = type;
		   this->reuse = reuse;
		   this->direction = direction;
		   this->val = val;
		   this->sock = sock;
        };

        ~NET_Connection ();

        int InitSocket(CipUdint family, CipUdint type, CipUdint protocol);
        int SetSocketOpt(CipUdint type, CipUdint reuse, CipUdint val);
        int BindSocket(int address_option, struct sockaddr * address);
        int Listen(int max_num_connections);

        void CloseSocket();

        int GetSocketHandle();
        int SetSocketHandle(int socket_handle);

        int SendData(void * data_ptr, CipUdint size);
        int RecvData (void *data_ptr, CipUdint size);
        int SendDataTo(void * data_ptr, CipUdint size, struct sockaddr * destination);
        int RecvDataFrom (void *data_ptr, CipUdint size, struct sockaddr * source);

    static std::map <int, NET_Connection*> socket_to_conn_map;

    // socket address for produce
    struct sockaddr *remote_address;

    // the address of the originator that established the connection. needed for
    // scanning if the right packet is arriving
    struct sockaddr *originator_address;

    // socket handles, indexed by kConsuming or kProducing
    int sock;


    // endianness conversion
    static uint32_t endian_htonl(uint32_t hostlong);
    static uint16_t endian_htons(uint16_t hostshort);
    static uint32_t endian_ntohl(uint32_t netlong);
    static uint16_t endian_ntohs(uint16_t netshort);


    private:
        static fd_set select_set[]; //0-master_socket 1-read_socket

        CipUdint type;
        CipUdint reuse;
        CipUdint direction;
        CipUdint val;

    //private functions
        //int CheckHandle(int handle);
};


#endif //OPENER_NET_CONNECTION_H
