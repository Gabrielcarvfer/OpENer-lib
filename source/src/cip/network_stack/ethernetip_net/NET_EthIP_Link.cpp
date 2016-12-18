/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <string.h>

#include <NET_EthIP_Link.h>

#include <connection_stack/CIP_Common.h>



void NET_EthIP_Link::ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_ethernet_link.physical_address, mac_address, sizeof(g_ethernet_link.physical_address));
}

CipStatus NET_EthIP_Link::CipEthernetLinkInit()
{
    if (instancesNum == 0)
    {
        /* set attributes to initial values */
        g_ethernet_link.interface_speed = 100;
        g_ethernet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */
        get_all_class_attributes_mask = 0xffffffff;
        get_all_instance_attributes_mask = 0xffffffff;
        class_name = "Ethernet Link";
        class_id = kCipEthernetLinkClassCode;


        NET_EthIP_Link *instance = new NET_EthIP_Link ();

        // bind attributes to the instance
        instance->InsertAttribute (1, kCipUdint, &g_ethernet_link.interface_speed, kGetableSingleAndAll);
        instance->InsertAttribute (2, kCipDword, &g_ethernet_link.interface_flags, kGetableSingleAndAll);
        instance->InsertAttribute (3, kCip6Usint, &g_ethernet_link.physical_address, kGetableSingleAndAll);
    }
    else
    {
        return kCipStatusError;
    }

    return kCipStatusOk;
}
