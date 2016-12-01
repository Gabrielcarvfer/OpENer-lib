/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifdef WIN32
#include <ws2tcpip.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>

#else
#include <string.h>
#include <sys/socket.h>
#endif

#include "src/cip/network_stack/networkhandler.h"

#include "src/cip/network_stack/ethernetip_net/eip_encap.h"
#include "src/cip/network_stack/generic_networkhandler.h"

MicroSeconds GetMicroSeconds()
{
    LARGE_INTEGER performance_counter;
    LARGE_INTEGER performance_frequency;

    QueryPerformanceCounter(&performance_counter);
    QueryPerformanceFrequency(&performance_frequency);

    return (MicroSeconds)(performance_counter.QuadPart * 1000000LL / performance_frequency.QuadPart);
}

MilliSeconds GetMilliSeconds(void)
{
    return (MilliSeconds)(GetMicroSeconds() / 1000ULL);
}

CipStatus NetworkHandlerInitializePlatform(void)
{
    /* Add platform dependent code here if necessary */
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif

    return kCipStatusOk;
}


