/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_CIPETHERNETLINK_H_
#define OPENER_CIPETHERNETLINK_H_

#include "ciptypes.h"
#include "typedefs.h"

#define CIP_ETHERNETLINK_CLASS_CODE 0xF6

/* public functions */
/** @brief Initialize the Ethernet Link Objects data
 */
CipStatus CipEthernetLinkInit(void);

#endif /* OPENER_CIPETHERNETLINK_H_*/
