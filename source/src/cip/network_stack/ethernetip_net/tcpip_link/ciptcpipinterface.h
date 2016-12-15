/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_CIPTCPIPINTERFACE_H_
#define OPENER_CIPTCPIPINTERFACE_H_

/** @file ciptcpipinterface.h
 * @brief Public interface of the TCP/IP Interface Object
 *
 */

#include "../../../ciptypes.h"
#include "../../../../typedefs.h"

extern CipTcpIpNetworkInterfaceConfiguration interface_configuration_;


/** @brief Multicast Configuration struct, called Mcast config
 *
 */
typedef struct multicast_address_configuration {
    CipUsint alloc_control; /**< 0 for default multicast address generation algorithm; 1 for multicast addresses according to Num MCast and MCast Start Addr */
    CipUsint reserved_shall_be_zero; /**< shall be zero */
    CipUint number_of_allocated_multicast_addresses; /**< Number of IP multicast addresses allocated */
    CipUdint starting_multicast_address; /**< Starting multicast address from which Num Mcast addresses are allocated */
} MulticastAddressConfiguration;

/* global public variables */
extern CipUsint g_time_to_live_value; /**< Time-to-live value for IP multicast packets. Default is 1; Minimum is 1; Maximum is 255 */

extern MulticastAddressConfiguration g_multicast_configuration; /**< Multicast configuration */

/* public functions */
/** @brief Initializing the data structures of the TCP/IP interface object
 */
CipStatus CipTcpIpInterfaceInit(void);

/** @brief Clean up the allocated data of the TCP/IP interface object.
 *
 * Currently this is the host name string and the domain name string.
 *
 */
void ShutdownTcpIpInterface(void);

#endif /* OPENER_CIPTCPIPINTERFACE_H_ */
