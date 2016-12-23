/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/

#include <stdio.h>
#include "eip_endianconv.hpp"
#include "../NET_Connection.hpp"
#include "../NET_Endianconv.hpp"

int EncapsulateIpAddress(CipUint port, CipUdint address, CipByte** communication_buffer)
{
    int size = 0;
    if (NET_Endianconv::kOpENerEndianessLittle == NET_Endianconv::g_opENer_platform_endianess)
    {
        size += NET_Endianconv::AddIntToMessage(NET_Connection::endian_htons (AF_INET), communication_buffer);
        size += NET_Endianconv::AddIntToMessage(port, communication_buffer);
        size += NET_Endianconv::AddDintToMessage(address, communication_buffer);

    }
    else
    {
        if (NET_Endianconv::kOpENerEndianessBig == NET_Endianconv::g_opENer_platform_endianess)
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
