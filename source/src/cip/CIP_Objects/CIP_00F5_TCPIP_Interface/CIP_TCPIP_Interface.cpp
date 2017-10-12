/******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 *****************************************************************************/

//Includes
#include <cstring>
#include "../../ciptypes.hpp"
#include "CIP_TCPIP_Interface.hpp"
#include "../../connection/network/NET_Connection.hpp"

#undef WIN
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define WIN
#endif

#ifdef WIN

    #include <iostream>
    #include <iphlpapi.h>
    #include <sstream>

    

	#pragma comment(lib, "iphlpapi.lib")
#elif __linux__
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <sys/ioctl.h>

#endif

//Defines
#define SZ(x) (sizeof(x))

CipUsint CIP_TCPIP_Interface::g_time_to_live_value;
CIP_TCPIP_Interface::multicast_address_configuration_t CIP_TCPIP_Interface::g_multicast_configuration;
CipTcpIpNetworkInterfaceConfiguration CIP_TCPIP_Interface::interface_configuration_;

//Methods
CipStatus CIP_TCPIP_Interface::ConfigureNetworkInterface(const char* ip_address,   const char* subnet_mask, const char* gateway)
{

    interface_configuration.ip_address = inet_addr(ip_address);
    interface_configuration.network_mask = inet_addr(subnet_mask);
    interface_configuration.gateway_address = inet_addr(gateway);

    // calculate the CIP multicast address. The multicast address is calculated, not input
    CipUdint host_id = NET_Connection::endian_ntohl(interface_configuration.ip_address) & ~NET_Connection::endian_ntohl(interface_configuration.network_mask); // see CIP spec 3-5.3 for multicast address algorithm
    host_id -= 1;
    host_id &= 0x3ff;

    g_multicast_configuration.starting_multicast_address = htonl(ntohl(inet_addr("239.192.1.0")) + (host_id << 5));

    return kCipGeneralStatusCodeSuccess;
}

void CIP_TCPIP_Interface::ConfigureDomainName(CipString * domain_name)
{
    if (nullptr != interface_configuration.domain_name.string)
    {
        // if the string is already set to a value we have to free the resources
        // before we can set the new value in order to avoid memory leaks.

        //CipFree(interface_configuration.domain_name.string);
        delete[] interface_configuration.domain_name.string;
    }
    interface_configuration.domain_name.length = domain_name->length;
    if (interface_configuration.domain_name.length)
    {
        interface_configuration.domain_name.string = new CipByte[interface_configuration.domain_name.length+1]();//(CipByte*)CipCalloc(interface_configuration.domain_name.length + 1, sizeof(EipInt8));
        strcpy((char*)interface_configuration.domain_name.string, (char*)domain_name->string);
    }
    else
    {
        interface_configuration.domain_name.string = nullptr;
    }
}

void CIP_TCPIP_Interface::ConfigureHostName(CipString * hostname)
{
    if (nullptr != hostname)
    {
        // if the string is already set to a value we have to free the resources
        // before we can set the new value in order to avoid memory leaks.

        delete[] host_name.string;
    }
    host_name.length = hostname->length;
    if (host_name.length)
    {
        host_name.string = new CipByte[host_name.length+1]();//(CipByte*)CipCalloc(hostname_.length + 1, sizeof(CipByte));
        strcpy((char*)host_name.string, (char*)hostname->string);
    }
    else
    {
        host_name.string = nullptr;
    }
}
/*
CipStatus CIP_TCPIP_Interface::SetAttributeSingleTcp(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)
{
    CIP_Attribute* attribute = this->GetCipAttribute(message_router_request->request_path.attribute_number);

    if (0 != attribute)
    {
        // it is an attribute we currently support, however no attribute is setable
        // TODO: if you like to have a device that can be configured via this CIP object add your code here
        // TODO: check for flags associated with attributes
        message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSetable;
    }
    else
    {
        // we don't have this attribute
        message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSupported;
    }

    message_router_response->size_additional_status = 0;
   message_router_response->response_data.clear();
    message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
    return kCipGeneralStatusCodeSuccess;
}
*/
CipStatus CIP_TCPIP_Interface::Init()
{
    CipStatus stat;


    if (number_of_instances == 0)
    {
        //Static variables
        class_name = "TCP/IP interface";
        class_id = kCipTcpIpInterfaceClassCode;
        revision = 0;
        g_time_to_live_value = 1;

        CIP_TCPIP_Interface *instance;
        instance = new CIP_TCPIP_Interface ();

        //Class attributes from Vol 2 Chapter 5
        instance->classAttrInfo.emplace (1, CipAttrInfo_t{ kCipUint, sizeof(CipUint), kAttrFlagGetableSingleAndAll, "Revision" });
        instance->classAttrInfo.emplace (2, CipAttrInfo_t{ kCipUint, sizeof(CipUint), kAttrFlagGetableSingleAndAll, "MaxInstances" });
        instance->classAttrInfo.emplace (3, CipAttrInfo_t{ kCipUint, sizeof(CipUint), kAttrFlagGetableSingleAndAll, "NumberOfInstances" });

        //Class optional attributes (4-7) from Vol 1 Chapter 4

        object_Set.emplace (instance->id, instance);
        
        //Setup instance attributes
        //Instance attributes
        //TODO: fix attribute access
        instance->instAttrInfo.emplace(1 , CipAttrInfo_t{ kCipDword                            , SZ(CipDword)                         , kAttrFlagGetableSingleAndAll, "" });
        instance->instAttrInfo.emplace(2 , CipAttrInfo_t{ kCipDword                            , SZ(CipDword)                         , kAttrFlagGetableSingleAndAll, "" });
        instance->instAttrInfo.emplace(3 , CipAttrInfo_t{ kCipDword                            , SZ(CipDword)                         , kAttrFlagSetAndGetAble      , "" });
        instance->instAttrInfo.emplace(4 , CipAttrInfo_t{ SZ(physical_link_object_t)           , SZ(physical_link_object_t)           , kAttrFlagGetableSingleAndAll, "PhysicalLinkObject" });
        instance->instAttrInfo.emplace(5 , CipAttrInfo_t{ SZ(interface_configuration_t)        , SZ(interface_configuration_t)        , kAttrFlagSetAndGetAble      , "InterfaceConfiguration" });
        instance->instAttrInfo.emplace(6 , CipAttrInfo_t{ kCipString                           , SZ(CipString)                        , kAttrFlagSetAndGetAble      , "" });
        //instance->classAttrInfo.emplace(7, //CipAttrInfo_t{ TODO:8 octets safety_network_number
        instance->instAttrInfo.emplace(8 , CipAttrInfo_t{ kCipUsint                            , SZ(CipUsint)                         , kAttrFlagSetAndGetAble      , "" });
        instance->instAttrInfo.emplace(9 , CipAttrInfo_t{ SZ(multicast_address_configuration_t), SZ(multicast_address_configuration_t), kAttrFlagSetAndGetAble      , "MulticastAddressConfiguration"});
        instance->instAttrInfo.emplace(10, CipAttrInfo_t{ kCipBool                             , SZ(CipBool)                          , kAttrFlagSetable            , "" });
        instance->instAttrInfo.emplace(11, CipAttrInfo_t{ SZ(last_conflict_detected_t)         , SZ(last_conflict_detected_t)         , kAttrFlagSetable            , "LastConflictDetected" });
        instance->instAttrInfo.emplace(12, CipAttrInfo_t{ kCipBool                             , SZ(CipBool)                          , kAttrFlagSetable            , "" });

        stat.status = kCipStatusOk;
    }
    else
    {
        stat.status = kCipStatusError;
    }

    return stat;
}

CipStatus CIP_TCPIP_Interface::Shut()
{
    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_TCPIP_Interface::Create()
{
    CIP_TCPIP_Interface* instance = new CIP_TCPIP_Interface();


    instance->status.value = 0x1;
    instance->configuration_capability.value = 0x04 | 0x20;
    instance->configuration_control.value = 0;
    instance->physical_link_object = {2, kCipTcpIpInterfaceClassCode, 1, 0};
    instance->interface_configuration = {0, 0, 0, 0, 0, {0, nullptr,}};
    //instance->host_name

    object_Set.emplace(instance->id, instance);

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    stat.extended_status = (CipUsint) instance->id;
    return stat;
}



void CIP_TCPIP_Interface::ShutdownTcpIpInterface(void)
{
    //Only free the resources if they are initialized
    /*if (nullptr != hostname_.string)
    {
        //CipFree(hostname_.string);
        delete[] hostname_.string;
        hostname_.string = nullptr;
    }*/

    /*if (nullptr != interface_configuration.domain_name.string)
    {
        //CipFree(interface_configuration.domain_name.string);
        interface_configuration.domain_name.string = nullptr;
    }*/
}

CipStatus CIP_TCPIP_Interface::GetAttributeSingleTcpIpInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)
{

    CipStatus status = kCipGeneralStatusCodeSuccess;
    CipByte* message = (CipByte*)&message_router_response->response_data[0];

    if (9 == message_router_request->request_path.attribute_number)
    { 
        // attribute 9 can not be easily handled with the default mechanism therefore we will do it by hand
       message_router_response->response_data.clear ();
        message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
        message_router_response->general_status = kCipGeneralStatusCodeSuccess;
        message_router_response->size_additional_status = 0;

        //message_router_response->response_data.size() += CIP_Common::EncodeData(kCipUsint, &(g_multicast_configuration.alloc_control), &message);
        //message_router_response->response_data.size() += CIP_Common::EncodeData(kCipUsint, &(g_multicast_configuration.reserved_shall_be_zero), &message);
        //message_router_response->response_data.size() += CIP_Common::EncodeData(kCipUint, &(g_multicast_configuration.number_of_allocated_multicast_addresses), &message);

        CipUdint multicast_address = ntohl(g_multicast_configuration.starting_multicast_address);

        //message_router_response->response_data.size() += CIP_Common::EncodeData(kCipUdint, &multicast_address, &message);
    }
    else
    {
        status = this->GetAttributeSingle(message_router_request, message_router_response);
    }
    return status;
}
/*
CipStatus CIP_TCPIP_Interface::GetAttributeAllTcpIpInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)
{

    CipUsint* response = (CipUsint*)&message_router_response->response_data[0]; // pointer into the reply
    CIP_Attribute* attribute;

    for (int j = 0; j < this->attributes.size(); j++) // for each instance attribute of this class
    {
        int attribute_number = attribute[j].getNumber ();

        // only return attributes that are flagged as being part of GetAttributeALl
        if (attribute_number < 32 )//todo:&& (this->get_all_instance_attributes_mask & 1 << attribute_number))
        {
            message_router_request->request_path.attribute_number = (CipUint) attribute_number;

            if (8 == attribute_number)
            {
                // insert 6 zeros for the required empty safety network number according to Table 5-3.10
                memset(&message_router_response->response_data[0], 0, 6);

                //message_router_response->response_data += 6;
            }

            if (kCipGeneralStatusCodeSuccess != this->GetAttributeSingleTcpIpInterface(message_router_request, message_router_response).status)
            {
                //message_router_response->response_data = response;
                return kCipStatusError;
            }
            //message_router_response->response_data += message_router_response->response_data.size();
        }
    }
    //message_router_response->response_data = response;

    return kCipGeneralStatusCodeSuccess;
}
*/


//Function scans for network interfaces and list them
//Function scans for network interfaces and list them
typedef struct {
    char ip[16];//15
    char netmask[16];//15
    char bcast_ip[16];//15

} ipv4_interface;

typedef struct {
    char ip[46];//45
    char netmask[46];//45
    char bcast_ip[46];//45
} ipv6_interface;

typedef struct {
    bool v4;
    char mac[18];
    bool up;
    bool p2p;
    bool loopback;
    bool bcast;
    bool mcast;
    union
    {
        ipv4_interface ipv4;
        ipv6_interface ipv6;
    };
} ip_interface;

int GetInterfacesList(ip_interface **interface_ptr);

//Scan for avaible tcpip interfaces
CipStatus CIP_TCPIP_Interface::ScanInterfaces()
{
    ip_interface ** interfaces = (ip_interface**)calloc(1,sizeof(ip_interface*));

    //Get interfaces list
    int numInterfaces = 0;
    numInterfaces = GetInterfacesList(interfaces);

    int i;
    for(i = 0; i < numInterfaces; i++)
    {
        //Search for an CIP_TCPIP_Interface object with same MAC address, if it exists, update info, if not, create a new object
        //(*interfaces)[i].mac
    }


    //Turn data list into CIP_TCPIP_Interface objects, checking before if the interface is already registered

    delete[] *interfaces;
    free(interfaces);
	return kCipGeneralStatusCodeSuccess;
}

#undef WIN
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define WIN
#endif

#ifdef WIN
//Windows version based on Silver Moon ( m00n.silv3r@gmail.com ) : http://www.binarytides.com/get-mac-address-from-ip-in-winsock/

//Functions
    void loadiphlpapi();
    DWORD GetMacAddress(char * , struct in_addr );

//Loads from Iphlpapi.dll
    typedef DWORD (WINAPI* psendarp)(struct in_addr DestIP, struct in_addr SrcIP, PULONG pMacAddr, PULONG PhyAddrLen );
    typedef DWORD (WINAPI* pgetadaptersinfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );

//2 functions
    psendarp SendArp;

    void loadiphlpapi()
    {
        HINSTANCE hDll = LoadLibrary("iphlpapi.dll");

        if(GetAdaptersInfo==nullptr)
        {
            printf("Error in iphlpapi.dll%d",(int)GetLastError());
        }

        SendArp = (psendarp)GetProcAddress(hDll,"SendARP");

        if(SendArp==nullptr)
        {
            printf("Error in iphlpapi.dll%d",(int)GetLastError());
        }
    }

/*
    Get the mac address of a given ip
*/
    DWORD GetMacAddress(char *mac , struct in_addr destip)
    {
        DWORD ret;
        struct in_addr srcip;
        ULONG MacAddr[2];
        ULONG PhyAddrLen = 6;  /* default to length of six bytes */
        int i;

        srcip.s_addr=0;

        //Send an arp packet
        ret = SendArp(destip , srcip , MacAddr , &PhyAddrLen);

        //Prepare the mac address
        if(PhyAddrLen == 6)
        {
            BYTE *bMacAddr = (BYTE *) & MacAddr;
            sprintf(mac, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",(CipUsint)bMacAddr[0], (CipUsint)bMacAddr[1], (CipUsint)bMacAddr[2], (CipUsint)bMacAddr[3], (CipUsint)bMacAddr[4], (CipUsint)bMacAddr[5]);

        }
        return ret;
    }
    int GetInterfacesList(ip_interface **interface_ptr)
    {
        WSADATA WinsockData;
        if (WSAStartup(MAKEWORD(2, 2), &WinsockData) != 0)
        {
            std::cerr << "Failed to find Winsock 2.2!" << std::endl;
            return 2;
        }

        SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
        if (sd == (unsigned)SOCKET_ERROR)
        {
            std::cerr << "Failed to get a socket. Error " << WSAGetLastError() << std::endl;
            return 1;
        }

        INTERFACE_INFO InterfaceList[20];
        unsigned long nBytesReturned;
        if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed calling WSAIoctl: error " << WSAGetLastError() << std::endl;
            return 1;
        }

        int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

        loadiphlpapi();

        *interface_ptr = new ip_interface[nNumInterfaces]();
        for (int i = 0; i < nNumInterfaces; ++i)
        {
            //Mark tcp/ip interface version
            (*interface_ptr)[i].v4 = true; //only ipv4 for now

            //Copy ip address
            strcpy(&((*interface_ptr)[i].ipv4.ip)[0], inet_ntoa(InterfaceList[i].iiAddress.AddressIn.sin_addr));

            //Copy MAC address
            GetMacAddress((*interface_ptr)[i].mac , InterfaceList[i].iiAddress.AddressIn.sin_addr);

            //Copy broadcast ip
            strcpy(&((*interface_ptr)[i].ipv4.bcast_ip)[0], inet_ntoa(InterfaceList[i].iiBroadcastAddress.AddressIn.sin_addr));

            //Copy netmask
            strcpy(&((*interface_ptr)[i].ipv4.netmask)[0], inet_ntoa(InterfaceList[i].iiNetmask.AddressIn.sin_addr));

            u_long nFlags = InterfaceList[i].iiFlags;
            (*interface_ptr)[i].up = (bool) (nFlags & IFF_UP);
            (*interface_ptr)[i].p2p = (bool) (nFlags & IFF_POINTTOPOINT);
            (*interface_ptr)[i].loopback = (bool) (nFlags & IFF_LOOPBACK);
            (*interface_ptr)[i].bcast = (bool) (nFlags & IFF_BROADCAST);
            (*interface_ptr)[i].mcast = (bool) (nFlags & IFF_MULTICAST);
        }

        //TODO: create TCPIP links and EthernetIP instances

        WSACleanup();
        return nNumInterfaces;
    }
#elif __linux__
int GetInterfacesList(ip_interface **interface_ptr)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s, i;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */
    for (ifa = ifaddr, i = 0; ifa != nullptr; ifa = ifa->ifa_next, i++)
    {
        if (ifa->ifa_addr == nullptr)
            continue;
    }

    *interface_ptr = new ip_interface[i]();

    for (ifa = ifaddr, i = 0; ifa != nullptr; ifa = ifa->ifa_next, i++)
    {
        if (ifa->ifa_addr == nullptr || ((family = ifa->ifa_addr->sa_family) != AF_INET))
            continue;

            s = getnameinfo(
                            ifa->ifa_addr,
                            sizeof(struct sockaddr_in),
                            host,
                            NI_MAXHOST,
                            nullptr,
                            0,
                            NI_NUMERICHOST
                            );
            if (s != 0)
            {
                //printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            int fd;
            unsigned char *mac_ptr = nullptr;
            char mac[18];
            struct ifreq ifr;

            memset(&ifr, 0, sizeof(ifr));
            memset(&mac[0],0,13);

            fd = socket(AF_INET, SOCK_DGRAM, 0);

            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name , ifa->ifa_name , IFNAMSIZ-1);

            ioctl(fd, SIOCGIFHWADDR, &ifr);
            mac_ptr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
            sprintf(&mac[0], "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", mac_ptr[0], mac_ptr[1], mac_ptr[2], mac_ptr[3], mac_ptr[4], mac_ptr[5]);

            printf("%s\t%s\tmac:<%s>\taddress: <%s>\n", ifa->ifa_name, "AF_INET", mac, host);

            //Mark tcp/ip interface version
        (*interface_ptr)[i].v4 = true;

        //Copy ip address
        strcpy(&((*interface_ptr)[i].ipv4.ip)[0], host);

        //Copy MAC address
        strcpy(&((*interface_ptr)[i].mac)[0], mac);

        //Copy broadcast ip
        inet_ntop( AF_INET, ifa->ifa_ifu.ifu_broadaddr, &((*interface_ptr)[i].ipv4.bcast_ip)[0], INET_ADDRSTRLEN);

        //Copy netmask
        inet_ntop( AF_INET,ifa->ifa_netmask, &((*interface_ptr)[i].ipv4.netmask)[0], INET_ADDRSTRLEN);

        //Copy flags
        u_long nFlags = (u_long) ifr.ifr_flags;
        (*interface_ptr)[i].up       = (bool) nFlags & IFF_UP;
        (*interface_ptr)[i].p2p      = (bool) nFlags & IFF_POINTOPOINT;
        (*interface_ptr)[i].loopback = (bool) nFlags & IFF_LOOPBACK;
        (*interface_ptr)[i].bcast    = (bool) nFlags & IFF_BROADCAST;
        (*interface_ptr)[i].mcast    = (bool) nFlags & IFF_MULTICAST;


    }

    freeifaddrs(ifaddr);
    return i;
}
#endif

void * CIP_TCPIP_Interface::retrieveAttribute(CipUsint attributeNumber)
{
	return nullptr;
}

CipStatus CIP_TCPIP_Interface::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{
    if (this->id == 0)
    {
        switch(serviceNumber)
        {
            case kServiceGetAttributeSingle: return this->GetAttributeSingleTcpIpInterface (req, resp);
            //case    kServiceGetAttributeAll: return this->GetAttributeAllTcpIpInterface (req, resp);
            //case kSetAttributeSingle: return this->SetAttributeSingleTcp (req, resp);
            default:
                return kCipStatusError;
        }

    }//Instance services
    else
    {
        return kCipStatusError;
    }
}