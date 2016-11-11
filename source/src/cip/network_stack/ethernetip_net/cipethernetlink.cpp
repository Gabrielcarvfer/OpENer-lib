/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <string.h>

#include "cipethernetlink.h"

#include "src/cip/connection_stack/cipcommon.h"
#include "ciperror.h"
#include "src/cip/connection_stack/cipmessagerouter.h"
#include "endianconv.h"
#include "opener_api.h"

typedef struct {
    CipUdint interface_speed;
    CipUdint interface_flags;
    CipUsint physical_address[6];
} CipEthernetLinkObject;

/* global private variables */
CipEthernetLinkObject g_ethernet_link;

void ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_ethernet_link.physical_address, mac_address,
        sizeof(g_ethernet_link.physical_address));
}

CipStatus CipEthernetLinkInit()
{
    CIP_Class* ethernet_link_class;
    CIP_Class* ethernet_link_instance;

    /* set attributes to initial values */
    g_ethernet_link.interface_speed = 100;
    g_ethernet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */

    if ((ethernet_link_class = CreateCIPClass(kCipEthernetLinkClassCode, 0, /* # class attributes*/
             0xffffffff, /* class getAttributeAll mask*/
             0, /* # class services*/
             3, /* # instance attributes*/
             0xffffffff, /* instance getAttributeAll mask*/
             0, /* # instance services*/
             1, /* # instances*/
             "Ethernet Link", 1))
        != 0) {

        ethernet_link_instance = GetCIPClass(ethernet_link_class, 1);
        InsertAttribute(ethernet_link_instance, 1, kCipUdint,
            &g_ethernet_link.interface_speed, kGetableSingleAndAll); /* bind attributes to the instance*/
        InsertAttribute(ethernet_link_instance, 2, kCipDword,
            &g_ethernet_link.interface_flags, kGetableSingleAndAll);
        InsertAttribute(ethernet_link_instance, 3, kCip6Usint,
            &g_ethernet_link.physical_address, kGetableSingleAndAll);
    } else {
        return kCipStatusError;
    }

    return kCipStatusOk;
}
