/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <signal.h>
#include <cstdio>
#include <cstdlib>

#include "src/cip/connection_stack/cipcommon.h"
#include "generic_networkhandler.h"
#include "opener_api.h"
#include "trace.h"

/******************************************************************************/
/** @brief Signal handler function for ending stack execution
 *
 * @param signal the signal we received
 */
void LeaveStack(int signal);

/*****************************************************************************/
/** @brief Flag indicating if the stack should end its execution
 */
int g_end_stack = 0;

/******************************************************************************/
int main(int argc, char* arg[])
{
    CipUsint my_mac_address[6];
    CipUint unique_connection_id;

    if (argc != 12) {
        printf("Wrong number of command line parameters!\n");
        printf("The correct command line parameters are:\n");
        printf(
            "./OpENer ipaddress subnetmask gateway domainname hostaddress macaddress\n");
        printf(
            "    e.g. ./OpENer 192.168.0.2 255.255.255.0 192.168.0.1 test.com testdevice 00 15 C5 BF D0 87\n");
        exit(0);
    } else {
        /* fetch Internet address info from the platform */
        ConfigureNetworkInterface(arg[1], arg[2], arg[3]);
        ConfigureDomainName(arg[4]);
        ConfigureHostName(arg[5]);

        my_mac_address[0] = (CipUsint)strtoul(arg[6], NULL, 16);
        my_mac_address[1] = (CipUsint)strtoul(arg[7], NULL, 16);
        my_mac_address[2] = (CipUsint)strtoul(arg[8], NULL, 16);
        my_mac_address[3] = (CipUsint)strtoul(arg[9], NULL, 16);
        my_mac_address[4] = (CipUsint)strtoul(arg[10], NULL, 16);
        my_mac_address[5] = (CipUsint)strtoul(arg[11], NULL, 16);
        ConfigureMacAddress(my_mac_address);
    }

    /*for a real device the serial number should be unique per device */
    SetDeviceSerialNumber(123456789);

    /* nUniqueConnectionID should be sufficiently random or incremented and stored
   *  in non-volatile memory each time the device boots.
   */
    unique_connection_id = rand();

    /* Setup the CIP Layer */
    CipStackInit(unique_connection_id);

    /* Setup Network Handles */
    if (kCipStatusOk == NetworkHandlerInitialize()) {
        g_end_stack = 0;
#ifndef WIN32
        /* register for closing signals so that we can trigger the stack to end */
        signal(SIGHUP, LeaveStack);
#endif

        /* The event loop. Put other processing you need done continually in here */
        while (1 != g_end_stack) {
            if (kCipStatusOk != NetworkHandlerProcessOnce()) {
                break;
            }
        }

        /* clean up network state */
        NetworkHandlerFinish();
    }
    /* close remaining sessions and connections, cleanup used data */
    ShutdownCipStack();

    return -1;
}

void LeaveStack(int signal)
{
    (void)signal; /* kill unused parameter warning */
    OPENER_TRACE_STATE("got signal HUP\n");
    g_end_stack = 1;
}
