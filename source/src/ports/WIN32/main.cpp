/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>

#include "src/cip/connection_stack/CIP_Common.h"
#include "src/cip/network_stack/NET_NetworkHandler.h"
#include "Opener_Interface.h"
#include "trace.h"

extern int newfd;

/******************************************************************************/
/*!\brief Signal handler function for ending stack execution
 *
 * @param pa_nSig the signal we received
 */
void LeaveStack(int pa_nSig);

/*****************************************************************************/


//Implement device specifit methods
CipStatus Opener_Interface::ResetDevice()
{

}

CipStatus Opener_Interface::ResetDeviceToInitialConfiguration()
{

}

/******************************************************************************/
int main(int argc, char* arg[])
{
    CipUsint acMyMACAddress[6];
    CipUint nUniqueConnectionID;

    if (argc != 12)
    {
        printf("Wrong number of command line parameters!\n");
        printf("The correct command line parameters are:\n");
        printf("./OpENer ipaddress subnetmask gateway domainname hostaddress macaddress\n");
        printf("    e.g. ./OpENer 192.168.0.2 255.255.255.0 192.168.0.1 test.com testdevice 00 15 C5 BF D0 87\n");
        exit(0);
    }
    else
    {
        // fetch Internet address info from the platform
        Opener_Interface::ConfigureNetworkInterface(arg[1], arg[2], arg[3]);
        Opener_Interface::ConfigureDomainName(arg[4]);
        Opener_Interface::ConfigureHostName(arg[5]);

        /*
        acMyMACAddress[0] = (CipUsint)std::strtoul(arg[6], NULL, 16);
        acMyMACAddress[1] = (CipUsint)std::strtoul(arg[7], NULL, 16);
        acMyMACAddress[2] = (CipUsint)std::strtoul(arg[8], NULL, 16);
        acMyMACAddress[3] = (CipUsint)std::strtoul(arg[9], NULL, 16);
        acMyMACAddress[4] = (CipUsint)std::strtoul(arg[10], NULL, 16);
        acMyMACAddress[5] = (CipUsint)std::strtoul(arg[11], NULL, 16);
        */

        Opener_Interface::ConfigureMacAddress(acMyMACAddress);
    }

    //for a real device the serial number should be unique per device
    Opener_Interface::SetDeviceSerialNumber(56789);

    // nUniqueConnectionID should be sufficiently random or incremented and stored
    // in non-volatile memory each time the device boots.

    nUniqueConnectionID = 56789;//rand();

    /* Setup the CIP Layer */
    Opener_Interface::CipStackInit(nUniqueConnectionID);

    /* Setup Network Handles */
    if (kCipStatusOk == NET_NetworkHandler::NetworkHandlerInitialize ())
    {
        g_end_stack = 0;
#ifndef WIN32
        /* register for closing signals so that we can trigger the stack to end */
        signal(SIGHUP, LeaveStack);
#endif

        /* The event loop. Put other processing you need done continually in here */
        while (1 != g_end_stack)
        {
            if (kCipStatusOk != NET_NetworkHandler::NetworkHandlerProcessOnce ())
            {
                break;
            }
        }

        /* clean up network state */
        NET_NetworkHandler::NetworkHandlerFinish();
    }
    /* close remaining sessions and connections, cleanup used data */
    Opener_Interface::ShutdownCipStack();

    return -1;
}

void LeaveStack(int pa_nSig)
{
    (void)pa_nSig; /* kill unused parameter warning */
    OPENER_TRACE_STATE("got signal HUP\n");
    g_end_stack = 1;
}
