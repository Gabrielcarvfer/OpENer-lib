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
        instance->instanceAttributesProperties.emplace(1, CipAttributeProperties_t{kCipUdint , sizeof(CipUdint)    , kGetableSingleAndAll, "InterfaceSpeed" } );
        instance->instanceAttributesProperties.emplace(2, CipAttributeProperties_t{kCipDword , sizeof(CipDword)    , kGetableSingleAndAll, "InterfaceFlags" } );
        instance->instanceAttributesProperties.emplace(3, CipAttributeProperties_t{kCip6Usint, sizeof(CipByteArray), kGetableSingleAndAll, "PhysicalAddress"} );

    }

    return kCipGeneralStatusCodeSuccess;
}

CipStatus CIP_EthernetIP_Link::Shut ()
{
	return kCipGeneralStatusCodeSuccess;
}

void * CIP_EthernetIP_Link::retrieveAttribute (CipUsint attributeNumber)
{
    if (this->id == 0)
    {
        switch(attributeNumber)
        {
            case 1: return &this->g_ethernet_link.interface_speed;
            case 2: return &this->g_ethernet_link.interface_flags;
            case 3: return &this->g_ethernet_link.physical_address;
            default: return nullptr;
        }
    }
}

CipStatus CIP_EthernetIP_Link::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{

}