/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "eip_endianconv.h"
#include "endianconv.h"

extern OpenerEndianess g_opener_platform_endianess;

int EncapsulateIpAddress(CipUint port, CipUdint address, CipByte** communication_buffer)
{
    int size = 0;
    if (kOpENerEndianessLittle == g_opener_platform_endianess)
    {
        size += AddIntToMessage(htons(AF_INET), (CipUsint**)communication_buffer);
        size += AddIntToMessage(port, communication_buffer);
        size += AddDintToMessage(address, communication_buffer);

    }
    else
    {
        if (kOpENerEndianessBig == g_opener_platform_endianess)
        {
            (*communication_buffer)[0] = (unsigned char)(AF_INET >> 8);
            (*communication_buffer)[1] = (unsigned char)AF_INET;
            *communication_buffer += 2;
            size += 2;

            (*communication_buffer)[0] = (unsigned char)(port >> 8);
            (*communication_buffer)[1] = (unsigned char)port;
            *communication_buffer += 2;
            size += 2;

            (*communication_buffer)[3] = (unsigned char)address;
            (*communication_buffer)[2] = (unsigned char)(address >> 8);
            (*communication_buffer)[1] = (unsigned char)(address >> 16);
            (*communication_buffer)[0] = (unsigned char)(address >> 24);
            *communication_buffer += 4;
            size += 4;
        }
        else
        {
            fprintf(stderr, "No endianess detected! Probably the DetermineEndianess function was not executed!");
            //exit(EXIT_FAILURE);
        }
    }
    return size;
}
