//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 25/01/17.
//

#ifndef CIP_CLASSES_TCPIPINTERFACE_H
#define CIP_CLASSES_TCPIPINTERFACE_H

/**
 * @brief Public interface of the TCP/IP Interface Object
 *
 */

#include "../template/CIP_Object.hpp"
#include "../../ciptypes.hpp"

class CIP_TCPIP_Interface;
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

    typedef struct
    {
        CipUint path_size;
        CipEpath padded_epath;
    }physical_link_object_t;

    typedef struct
    {
        CipUdint ip_address;
        CipUdint network_mask;
        CipUdint gateway_address;
        CipUdint name_server;
        CipUdint name_server2;
        CipString domain_name;
    }interface_configuration_t;

    typedef struct
    {
        CipUsint acd_activity;
        CipUsint remote_mac[6];
        CipUsint arp_pdu[28];
    } last_conflict_detected_t;

    typedef enum
    {
        not_configured = 0,
        valid_software_configuration = 1,
        valid_hardware_configuration = 2
        //  3 (0x3) to 15 (0xF) is reserved
    }instance_status_interface_configuration_status_e;

    typedef union
    {
        struct
        {
            CipUdint interface_configuration_status:4;
            CipUdint mcast_pending: 1;
            CipUdint interface_configuration_pending:1;
            CipUdint acd_status:1;
            CipUdint reserved:25;
        }bitfield_u;
        CipUdint value;
    }instance_status_t;

    typedef union
    {
        CipUdint value;
        struct
        {
            CipUdint bootp_client:1;
            CipUdint dns_client:1;
            CipUdint dhcp_client:1;
            CipUdint dhcp_dns_update:1;
            CipUdint configuration_settable:1;
            CipUdint hardware_configurable:1;
            CipUdint interface_change_requires_reset:1;
            CipUdint acd_capable:1;
            CipUdint reserved:24;
        }bitfield_u;
    }configuration_capability_attributes_t;

    typedef enum
    {
        //configuration methods bits 3-0
        statical_config = 0,
        bootp_config    = 1,
        dhcp_config     = 2
        //  3 (0x3) to 15 (0xF) is reserved
    }configuration_control_attributes_e;

    typedef union
    {

        CipUdint value;
        struct
        {
            CipUdint configuration_method:4;
            CipUdint dns_enable:1;
            CipUdint reserved:27;
        }bitfield_u;
    }configuration_control_attributes_t;



/* public functions */
/** @brief Initializing the data structures of the TCP/IP interface object
 */
    static CipStatus Init();
    static CipStatus Shut();
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
    instance_status_t status;
    configuration_capability_attributes_t configuration_capability;
    configuration_control_attributes_t configuration_control;
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
    CipStatus GetAttributeSingleTcpIpInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);

    //CipStatus GetAttributeAllTcpIpInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);

    CipStatus ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway);
    void ConfigureDomainName(CipString * domain_name);
    void ConfigureHostName(CipString * hostname);
    //CipStatus SetAttributeSingleTcp(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);


    void * retrieveAttribute(CipUsint attributeNumber);
    CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
};
#endif /* CIP_CLASSES_TCPIPINTERFACE_H */
