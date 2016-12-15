/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/

#include <stdio.h>
#include "eip_endianconv.h"
#include "../../../utils/UTIL_Endianconv.h"
#include "../NET_Connection.h"


#ifdef WIN32
#include <winsock2.h>
#else
#endif

int EncapsulateIpAddress(CipUint port, CipUdint address, CipByte** communication_buffer)
{
    int size = 0;
    if (UTIL_Endianconv::kOpENerEndianessLittle == UTIL_Endianconv::g_opener_platform_endianess)
    {
        size += UTIL_Endianconv::AddIntToMessage(NET_Connection::endian_htons (AF_INET), (CipUsint**)communication_buffer);
        size += UTIL_Endianconv::AddIntToMessage(port, communication_buffer);
        size += UTIL_Endianconv::AddDintToMessage(address, communication_buffer);

    }
    else
    {
        if (UTIL_Endianconv::kOpENerEndianessBig == UTIL_Endianconv::g_opener_platform_endianess)
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
