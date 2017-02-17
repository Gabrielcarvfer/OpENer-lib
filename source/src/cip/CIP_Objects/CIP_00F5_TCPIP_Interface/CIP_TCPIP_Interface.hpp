//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#ifndef OPENER_CIPTCPIPINTERFACE_H_
#define OPENER_CIPTCPIPINTERFACE_H_

/**
 * @brief Public interface of the TCP/IP Interface Object
 *
 */

#include "../template/CIP_Object.hpp"
#include "../../ciptypes.hpp"

class CIP_TCPIP_Interface : public CIP_Object<CIP_TCPIP_Interface>
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
    } multicast_address_configuration_t;

    typedef struct {
        CipUint path_size;
        CipEpath padded_epath;
    }physical_link_object_t;

    typedef struct {
        CipUdint ip_address;
        CipUdint network_mask;
        CipUdint gateway_address;
        CipUdint name_server;
        CipUdint name_server2;
        CipString domain_name;
    }interface_configuration_t;

    typedef struct {
        CipUsint acd_activity;
        CipUsint remote_mac[6];
        CipUsint arp_pdu[28];
    } last_conflict_detected_t;

    typedef enum
    {
        //interface configuration status bits 3-0
        not_configured               = 0x00,
        valid_software_configuration = 0x01,
        valid_hardware_configuration = 0x02,

        //other configuration
        mcast_pending                   = BIT_N(4),
        interface_configuration_pending = BIT_N(5),
        acd_status                      = BIT_N(6)
    }instance_status;

    typedef enum {
        bootp_client                    = BIT_N(0),
        dns_client                      = BIT_N(1),
        dhcp_client                     = BIT_N(2),
        dhcp_dns_update                 = BIT_N(3),
        configuration_settable          = BIT_N(4),
        hardware_configurable           = BIT_N(5),
        interface_change_requires_reset = BIT_N(6),
        acd_capable                     = BIT_N(7)
    }configuration_capability_attributes;

    typedef enum {
        //configuration methods bits 3-0
        statical_config = 0x00,
        bootp_config    = 0x01,
        dhcp_config     = 0x02,

        //other configuration
        dns_enable = BIT_N(4)
    }configuration_control_attributes;



/* public functions */
/** @brief Initializing the data structures of the TCP/IP interface object
 */
    static CipStatus Init();
    static CipStatus Create();

/** @brief Clean up the allocated data of the TCP/IP interface object.
 * Currently this is the host name string and the domain name string.
 */
    static void ShutdownTcpIpInterface (void);

    /** @brief #9 The multicast configuration for this device
 *
 * Currently we implement it non set-able and use the default
 * allocation algorithm
 */
    static multicast_address_configuration_t g_multicast_configuration;
    /*< #5 IP, network mask, gateway, name server 1 & 2, domain name*/
    static CipTcpIpNetworkInterfaceConfiguration interface_configuration_;
    /** @brief #8 the time to live value to be used for multi-cast connections
 *
 * Currently we implement it non set-able and with the default value of 1.
 */
    static CipUsint g_time_to_live_value;
    static CipStatus ScanInterfaces();

private:
    //Instance attributes
    CipDword status;
    CipDword configuration_capability;
    CipDword configuration_control;
    physical_link_object_t physical_link_object;
    interface_configuration_t interface_configuration;
    CipString host_name;
    //TODO:8 octets safety_network_number
    CipUsint ttl_value;
    multicast_address_configuration_t multicast_address_configuration;
    CipBool select_acd;
    last_conflict_detected_t last_conflict_detected;
    CipBool quick_connect;

    //Functions
    CipStatus InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response);

    //Functions
    CipStatus GetAttributeSingleTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

    CipStatus GetAttributeAllTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

    CipStatus ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway);
    void ConfigureDomainName(CipString * domain_name);
    void ConfigureHostName(CipString * hostname);
    CipStatus SetAttributeSingleTcp(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);

};
#endif /* OPENER_CIPTCPIPINTERFACE_H_ */
