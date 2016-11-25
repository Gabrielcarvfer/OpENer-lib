/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <string.h>

#include "ciptcpipinterface.h"

#include "src/cip/connection_stack/CIP_Common.h"
#include "ciperror.h"
#include "src/cip/network_stack/ethernetip_net/cipethernetlink.h"
#include "src/cip/connection_stack/CIP_MessageRouter.h"
#include "endianconv.h"
#include "opener_api.h"
#include "opener_user_conf.h"

CipDword tcp_status_ = 0x1; /**< #1  TCP status with 1 we indicate that we got a valid configuration from DHCP or BOOTP */
CipDword configuration_capability_ = 0x04 | 0x20; /**< #2  This is a default value meaning that it is a DHCP client see 5-3.2.2.2 EIP specification; 0x20 indicates "Hardware Configurable" */
CipDword configuration_control_ = 0; /**< #3  This is a TCP/IP object attribute. For now it is always zero and is not used for anything. */
CipEpath physical_link_object_ = /**< #4 */
    {
      2, /**< EIP_UINT16 (UINT) PathSize in 16 Bit chunks*/
      CIP_ETHERNETLINK_CLASS_CODE, /**< EIP_UINT16 ClassID*/
      1, /**< EIP_UINT16 InstanceNr*/
      0 /**< EIP_UINT16 AttributNr (not used as this is the EPATH the EthernetLink object)*/
    };

CipTcpIpNetworkInterfaceConfiguration interface_configuration_ = /**< #5 IP, network mask, gateway, name server 1 & 2, domain name*/
    { 0, /* default IP address */
      0, /* NetworkMask */
      0, /* Gateway */
      0, /* NameServer */
      0, /* NameServer2 */
      {
          /* DomainName */
          0, NULL,
      } };

CipString hostname_ = /**< #6 Hostname*/
    { 0, NULL };
/** @brief #8 the time to live value to be used for multi-cast connections
 *
 * Currently we implement it non set-able and with the default value of 1.
 */
CipUsint g_time_to_live_value = 1;

/** @brief #9 The multicast configuration for this device
 *
 * Currently we implement it non set-able and use the default
 * allocation algorithm
 */
MulticastAddressConfiguration g_multicast_configuration = {
    0, /* us the default allocation algorithm */
    0, /* reserved */
    1, /* we currently use only one multicast address */
    0 /* the multicast address will be allocated on ip address configuration */
};

/************** Functions ****************************************/
CipStatus GetAttributeSingleTcpIpInterface(
    CIP_ClassInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response);

CipStatus GetAttributeAllTcpIpInterface(
    CIP_ClassInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response);

CipStatus ConfigureNetworkInterface(const char* ip_address,
    const char* subnet_mask,
    const char* gateway)
{

    interface_configuration_.ip_address = inet_addr(ip_address);
    interface_configuration_.network_mask = inet_addr(subnet_mask);
    interface_configuration_.gateway = inet_addr(gateway);

    /* calculate the CIP multicast address. The multicast address is calculated, not input*/
    CipUdint host_id = ntohl(interface_configuration_.ip_address)
        & ~ntohl(interface_configuration_.network_mask); /* see CIP spec 3-5.3 for multicast address algorithm*/
    host_id -= 1;
    host_id &= 0x3ff;

    g_multicast_configuration.starting_multicast_address = htonl(
        ntohl(inet_addr("239.192.1.0")) + (host_id << 5));

    return kCipStatusOk;
}

void ConfigureDomainName(const char* domain_name)
{
    if (NULL != interface_configuration_.domain_name.string) {
        /* if the string is already set to a value we have to free the resources
     * before we can set the new value in order to avoid memory leaks.
     */
        CipFree(interface_configuration_.domain_name.string);
    }
    interface_configuration_.domain_name.length = strlen(domain_name);
    if (interface_configuration_.domain_name.length) {
        interface_configuration_.domain_name.string = (EipByte*)CipCalloc(
            interface_configuration_.domain_name.length + 1, sizeof(EipInt8));
        strcpy((char*)interface_configuration_.domain_name.string, domain_name);
    } else {
        interface_configuration_.domain_name.string = NULL;
    }
}

void ConfigureHostName(const char* hostname)
{
    if (NULL != hostname_.string) {
        /* if the string is already set to a value we have to free the resources
     * before we can set the new value in order to avoid memory leaks.
     */
        CipFree(hostname_.string);
    }
    hostname_.length = strlen(hostname);
    if (hostname_.length) {
        hostname_.string = (EipByte*)CipCalloc(hostname_.length + 1,
            sizeof(EipByte));
        strcpy((char*)hostname_.string, hostname);
    } else {
        hostname_.string = NULL;
    }
}

CipStatus SetAttributeSingleTcp(
    CIP_ClassInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    CIP_Attribute* attribute = GetCipAttribute(
        instance, message_router_request->request_path.attribute_number);
    (void)instance; /*Suppress compiler warning */

    if (0 != attribute) {
        /* it is an attribute we currently support, however no attribute is setable */
        /* TODO: if you like to have a device that can be configured via this CIP object add your code here */
        /* TODO: check for flags associated with attributes */
        message_router_response->general_status = kCipErrorAttributeNotSetable;
    } else {
        /* we don't have this attribute */
        message_router_response->general_status = kCipErrorAttributeNotSupported;
    }

    message_router_response->size_of_additional_status = 0;
    message_router_response->data_length = 0;
    message_router_response->reply_service = (0x80
        | message_router_request->service);
    return kCipStatusOkSend;
}

CipStatus CipTcpIpInterfaceInit()
{
    CIP_ClassInstance* tcp_ip_class = NULL;

    if ((tcp_ip_class = CreateCIPClass(kCipTcpIpInterfaceClassCode, 0, /* # class attributes*/
             0xffffffff, /* class getAttributeAll mask*/
             0, /* # class services*/
             8, /* # instance attributes*/
             0xffffffff, /* instance getAttributeAll mask*/
             1, /* # instance services*/
             1, /* # instances*/
             "TCP/IP interface", 3))
        == 0) {
        return kCipStatusError;
    }
    CIP_ClassInstance* instance = GetCIPClass(tcp_ip_class, 1); /* bind attributes to the instance #1 that was created above*/

    InsertAttribute(instance, 1, kCipDword, (void*)&tcp_status_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 2, kCipDword, (void*)&configuration_capability_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 3, kCipDword, (void*)&configuration_control_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 4, kCipEpath, &physical_link_object_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 5, kCipUdintUdintUdintUdintUdintString,
        &interface_configuration_, kGetableSingleAndAll);
    InsertAttribute(instance, 6, kCipString, (void*)&hostname_,
        kGetableSingleAndAll);

    InsertAttribute(instance, 8, kCipUsint, (void*)&g_time_to_live_value,
        kGetableSingleAndAll);
    InsertAttribute(instance, 9, kCipAny, (void*)&g_multicast_configuration,
        kGetableSingleAndAll);

    InsertService(tcp_ip_class, kGetAttributeSingle,
        &GetAttributeSingleTcpIpInterface,
        "GetAttributeSingleTCPIPInterface");

    InsertService(tcp_ip_class, kGetAttributeAll, &GetAttributeAllTcpIpInterface,
        "GetAttributeAllTCPIPInterface");

    InsertService(tcp_ip_class, kSetAttributeSingle, &SetAttributeSingleTcp,
        "SetAttributeSingle");

    return kCipStatusOk;
}

void ShutdownTcpIpInterface(void)
{
    /*Only free the resources if they are initialized */
    if (NULL != hostname_.string) {
        CipFree(hostname_.string);
        hostname_.string = NULL;
    }

    if (NULL != interface_configuration_.domain_name.string) {
        CipFree(interface_configuration_.domain_name.string);
        interface_configuration_.domain_name.string = NULL;
    }
}

CipStatus GetAttributeSingleTcpIpInterface(
    CIP_ClassInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{

    CipStatus status = kCipStatusOkSend;
    EipByte* message = message_router_response->data;

    if (9 == message_router_request->request_path.attribute_number) { /* attribute 9 can not be easily handled with the default mechanism therefore we will do it by hand */
        message_router_response->data_length = 0;
        message_router_response->reply_service = (0x80
            | message_router_request->service);
        message_router_response->general_status = kCipErrorSuccess;
        message_router_response->size_of_additional_status = 0;

        message_router_response->data_length += EncodeData(
            kCipUsint, &(g_multicast_configuration.alloc_control), &message);
        message_router_response->data_length += EncodeData(
            kCipUsint, &(g_multicast_configuration.reserved_shall_be_zero),
            &message);
        message_router_response->data_length += EncodeData(
            kCipUint,
            &(g_multicast_configuration.number_of_allocated_multicast_addresses),
            &message);

        CipUdint multicast_address = ntohl(
            g_multicast_configuration.starting_multicast_address);

        message_router_response->data_length += EncodeData(kCipUdint,
            &multicast_address,
            &message);
    } else {
        status = GetAttributeSingle(instance, message_router_request,
            message_router_response);
    }
    return status;
}

CipStatus GetAttributeAllTcpIpInterface(
    CIP_ClassInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{

    CipUsint* response = message_router_response->data; /* pointer into the reply */
    CIP_Attribute* attribute = instance->attributes;

    for (int j = 0; j < instance->CIP_ClassInstance->number_of_attributes; j++) /* for each instance attribute of this class */
    {
        int attribute_number = attribute->attribute_number;
        if (attribute_number < 32
            && (instance->CIP_ClassInstance->get_attribute_all_mask & 1 << attribute_number)) /* only return attributes that are flagged as being part of GetAttributeALl */
        {
            message_router_request->request_path.attribute_number = attribute_number;

            if (8 == attribute_number) { /* insert 6 zeros for the required empty safety network number according to Table 5-3.10 */
                memset(message_router_response->data, 0, 6);
                message_router_response->data += 6;
            }

            if (kCipStatusOkSend
                != GetAttributeSingleTcpIpInterface(instance, message_router_request,
                       message_router_response)) {
                message_router_response->data = response;
                return kCipStatusError;
            }
            message_router_response->data += message_router_response->data_length;
        }
        attribute++;
    }
    message_router_response->data_length = message_router_response->data
        - response;
    message_router_response->data = response;

    return kCipStatusOkSend;
}
