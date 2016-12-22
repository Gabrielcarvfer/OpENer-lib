/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <cstring>
#include "NET_EthIP_Link.hpp"


void NET_EthIP_Link::ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_ethernet_link.physical_address, mac_address, sizeof(g_ethernet_link.physical_address));
}

CipStatus NET_EthIP_Link::CipEthernetLinkInit()
{
    if (instancesNum == 0)
    {
        get_all_class_attributes_mask = 0xffffffff;
        get_all_instance_attributes_mask = 0xffffffff;
        class_name = "Ethernet Link";
        class_id = kCipEthernetLinkClassCode;
    }
        NET_EthIP_Link * instance = new NET_EthIP_Link();
        /* set attributes to initial values */
        instance->g_ethernet_link.interface_speed = 100;
        instance->g_ethernet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */


        // bind attributes to the instance
        instance->InsertAttribute (1, kCipUdint, &instance->g_ethernet_link.interface_speed, kGetableSingleAndAll);
        instance->InsertAttribute (2, kCipDword, &instance->g_ethernet_link.interface_flags, kGetableSingleAndAll);
        instance->InsertAttribute (3, kCip6Usint, &instance->g_ethernet_link.physical_address, kGetableSingleAndAll);


    NET_EthIP_Interface::CipTcpIpInterfaceInit ();

    return kCipStatusOk;
}

CipStatus NET_EthIP_Link::Shutdown ()
{
    NET_EthIP_Interface::ShutdownTcpIpInterface ();
}

CipStatus NET_EthIP_Link::InstanceServices(int service, CipMessageRouterRequest * msg_router_request,CipMessageRouterResponse* msg_router_response)
{
    return kCipStatusError;
}
