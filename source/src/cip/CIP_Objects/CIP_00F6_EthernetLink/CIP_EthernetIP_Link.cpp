/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <cstring>
#include "CIP_EthernetIP_Link.hpp"


void CIP_EthernetIP_Link::ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_ethernet_link.physical_address, mac_address, sizeof(g_ethernet_link.physical_address));
}

CipStatus CIP_EthernetIP_Link::Init()
{
    if (number_of_instances == 0)
    {
        //Init class attributes from 1 to 7
        revision = kEthIPMultiplePortRev;
        //max_inst
        //num_inst
        class_name = "Ethernet Link";
        class_id = kCipEthernetLinkClassCode;

        CIP_EthernetIP_Link * instance = new CIP_EthernetIP_Link();
        /* set attributes to initial values */
        instance->g_ethernet_link.interface_speed = 100;
        instance->g_ethernet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */


        // bind attributes to the instance
        instance->InsertAttribute (1, kCipUdint, &instance->g_ethernet_link.interface_speed, kGetableSingleAndAll);
        instance->InsertAttribute (2, kCipDword, &instance->g_ethernet_link.interface_flags, kGetableSingleAndAll);
        instance->InsertAttribute (3, kCip6Usint, &instance->g_ethernet_link.physical_address, kGetableSingleAndAll);
    }
    return kCipStatusOk;
}

CipStatus CIP_EthernetIP_Link::Shutdown ()
{
	return kCipStatusOk;
}

CipStatus CIP_EthernetIP_Link::InstanceServices(int service, CipMessageRouterRequest * msg_router_request,CipMessageRouterResponse* msg_router_response)
{
    return kCipStatusError;
}
