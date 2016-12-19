/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIPETHERNETLINK_H_
#define OPENER_CIPETHERNETLINK_H_

#include <ciptypes.h>
#include <typedefs.h>
#include <CIP_Object.h>

class NET_EthIP_Link : public CIP_Object
{
public:
    /** @brief Initialize the Ethernet Link Objects data
    */
    static CipStatus CipEthernetLinkInit(void);
private:
    //Definitions
    typedef struct {
        CipUdint interface_speed;
        CipUdint interface_flags;
        CipUsint physical_address[6];
    } CipEthernetLinkObject;

    //Methods
    static void ConfigureMacAddress(const CipUsint* mac_address);

    //Variables
    static CipEthernetLinkObject g_ethernet_link;
};

#endif /* OPENER_CIPETHERNETLINK_H_*/
