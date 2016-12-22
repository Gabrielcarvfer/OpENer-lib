/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIPTCPIPINTERFACE_H_
#define OPENER_CIPTCPIPINTERFACE_H_

/** @file ciptcpipinterface.h
 * @brief Public interface of the TCP/IP Interface Object
 *
 */

#include "../../../../template/CIP_Object.hpp"
#include "../../../../ciptypes.hpp"

class NET_EthIP_Interface : public CIP_Object<NET_EthIP_Interface>
{
public:
/** @brief Multicast Configuration struct, called Mcast config
 */
    typedef struct multicast_address_configuration
    {
        CipUsint alloc_control; /**< 0 for default multicast address generation algorithm; 1 for multicast addresses according to Num MCast and MCast Start Addr */
        CipUsint reserved_shall_be_zero; /**< shall be zero */
        CipUint number_of_allocated_multicast_addresses; /**< Number of IP multicast addresses allocated */
        CipUdint starting_multicast_address; /**< Starting multicast address from which Num Mcast addresses are allocated */
    } MulticastAddressConfiguration;

/* global public variables */
    //extern CipUsint g_time_to_live_value; /**< Time-to-live value for IP multicast packets. Default is 1; Minimum is 1; Maximum is 255 */

    //extern MulticastAddressConfiguration g_multicast_configuration; /**< Multicast configuration */

/* public functions */
/** @brief Initializing the data structures of the TCP/IP interface object
 */
    static CipStatus CipTcpIpInterfaceInit (void);

/** @brief Clean up the allocated data of the TCP/IP interface object.
 * Currently this is the host name string and the domain name string.
 */
    static void ShutdownTcpIpInterface (void);

    /** @brief #9 The multicast configuration for this device
 *
 * Currently we implement it non set-able and use the default
 * allocation algorithm
 */
    static MulticastAddressConfiguration g_multicast_configuration;
    /*< #5 IP, network mask, gateway, name server 1 & 2, domain name*/
    static CipTcpIpNetworkInterfaceConfiguration interface_configuration_;
    /** @brief #8 the time to live value to be used for multi-cast connections
 *
 * Currently we implement it non set-able and with the default value of 1.
 */
    static CipUsint g_time_to_live_value;
    static CipStatus ScanInterfaces();

private:
    CipStatus InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response);

    //< #1  TCP status with 1 we indicate that we got a valid configuration from DHCP or BOOTP
    static CipDword tcp_status_;

    //< #2  This is a default value meaning that it is a DHCP client see 5-3.2.2.2 EIP specification; 0x20 indicates "Hardware Configurable"
    static CipDword configuration_capability_;

    //< #3  This is a TCP/IP object attribute. For now it is always zero and is not used for anything.
    static CipDword configuration_control_;

    //< #4 {< EIP_UINT16 (UINT) PathSize in 16 Bit chunks, < EIP_UINT16 ClassID, < EIP_UINT16 InstanceNr, < EIP_UINT16 AttributNr (not used as this is the EPATH the EthernetLink object)}
    static CipEpath physical_link_object_;

    /**< #6 Hostname*/
    static CipString hostname_;

    //Functions
    CipStatus GetAttributeSingleTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

    CipStatus GetAttributeAllTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

    CipStatus ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway);
    void ConfigureDomainName(const char* domain_name);
    void ConfigureHostName(const char* hostname);
    CipStatus SetAttributeSingleTcp(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

};
#endif /* OPENER_CIPTCPIPINTERFACE_H_ */
