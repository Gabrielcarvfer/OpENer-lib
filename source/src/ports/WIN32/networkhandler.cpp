/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include <Ws2tcpip.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>

#include "networkhandler.h"

#include "../enet_encap/eip_encap.h"
#include "generic_networkhandler.h"

MicroSeconds getMicroSeconds()
{
    LARGE_INTEGER performance_counter;
    LARGE_INTEGER performance_frequency;

    QueryPerformanceCounter(&performance_counter);
    QueryPerformanceFrequency(&performance_frequency);

    return (MicroSeconds)(performance_counter.QuadPart * 1000000LL
        / performance_frequency.QuadPart);
}

MilliSeconds GetMilliSeconds(void)
{
    return (MilliSeconds)(getMicroSeconds() / 1000ULL);
}

CipStatus NetworkHandlerInitializePlatform(void)
{
    /* Add platform dependent code here if necessary */
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);

    return kCipStatusOk;
}

void CloseSocketPlatform(int socket_handle)
{
    closesocket(socket_handle);
}
