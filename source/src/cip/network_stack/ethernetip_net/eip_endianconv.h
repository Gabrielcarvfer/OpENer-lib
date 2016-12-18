/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_ENET_ENDIANCONV_H_
#define OPENER_ENET_ENDIANCONV_H_

#include <typedefs.h>

/** @brief Encapsulate the sockaddr information as necessary for the Common Packet Format data items
 *
 * Converts and adds the provided port and IP address into an common packet format message
 *
 * @param port Port of the socket, has to be provided in big-endian
 * @param address IP address of the socket, has to be provided in big-endian
 * @param communcation_buffer The message buffer for sending the message
 */
int EncapsulateIpAddress(CipUint port, CipUdint address,
    CipByte** communication_buffer);

#endif /* OPENER_ENDIANCONV_H_ */
