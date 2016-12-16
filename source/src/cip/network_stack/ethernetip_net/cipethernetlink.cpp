/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <string.h>

#include "cipethernetlink.h"

#include "../../connection_stack/CIP_Common.h"

typedef struct {
    CipUdint interface_speed;
    CipUdint interface_flags;
    CipUsint physical_address[6];
} CipEthernetLinkObject;

/* global private variables */
CipEthernetLinkObject g_ethernet_link;

void ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_ethernet_link.physical_address, mac_address, sizeof(g_ethernet_link.physical_address));
}

CipStatus CipEthernetLinkInit()
{
    CIP_ClassInstance* ethernet_link_class;
    CIP_ClassInstance* ethernet_link_instance;

    /* set attributes to initial values */
    g_ethernet_link.interface_speed = 100;
    g_ethernet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */

    if ((ethernet_link_class = new CIP_ClassInstance(kCipEthernetLinkClassCode,
                                                             0xffffffff, /* class getAttributeAll mask*/
                                                             0xffffffff, /* instance getAttributeAll mask*/
                                                             "Ethernet Link", 1))
                                                        != 0)
    {

        ethernet_link_instance = new CIP_ClassInstance(kCipEthernetLinkClassCode,0,0,std::string(""),1);

        // bind attributes to the instance
        ethernet_link_instance->InsertAttribute(1, kCipUdint,  &g_ethernet_link.interface_speed,  kGetableSingleAndAll);
        ethernet_link_instance->InsertAttribute(2, kCipDword,  &g_ethernet_link.interface_flags,  kGetableSingleAndAll);
        ethernet_link_instance->InsertAttribute(3, kCip6Usint, &g_ethernet_link.physical_address, kGetableSingleAndAll);
    }
    else
    {
        return kCipStatusError;
    }

    return kCipStatusOk;
}
