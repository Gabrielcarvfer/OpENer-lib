//
// Created by gabriel on 11/11/2016.
//

#include "Opener_Interface.h"
#include <stdlib.h>


bool Opener_Interface::Opener_Initialize(CipUdint serialNumber)
{
    CipUint unique_connection_id;

    /*for a real device the serial number should be unique per device */
    SetDeviceSerialNumber(123456789);

    /* nUniqueConnectionID should be sufficiently random or incremented and stored
    *  in non-volatile memory each time the device boots.
    */
    unique_connection_id = rand();

    /* Setup the CIP Layer */
    CipStackInit(unique_connection_id);

    /* Setup Network Handles */
    if (kCipStatusOk == NetworkHandlerInitialize ())
    {
        g_end_stack = 0;
#ifndef WIN32
        /* register for closing signals so that we can trigger the stack to end */
                signal(SIGHUP, LeaveStack);
#endif

        /* The event loop. Put other processing you need done continually in here */
        while (1 != g_end_stack)
        {
            if (kCipStatusOk != NetworkHandlerProcessOnce ())
            {
                break;
            }
        }
    }
}

bool Opener_Interface::Opener_Shutdown()
{
    //TODO: clean up all Explicit and IO connections

    //TODO: if cipstatusok, finish handler
    // clean up network state
    NetworkHandlerFinish ();

    // close remaining sessions and connections, cleanup used data
    ShutdownCipStack();
}

//Open a new Explicit connection
CipUdint Opener_Interface::Opener_CreateExplicitConnection()
{
    CipUdint handle = Explicit_Connection_set.size();
    Explicit_Connection_set.emplace(handle, new Opener_ExplicitConnection());
    return handle;
}

//Destroy Explicit connection and then close it, not in this order
void Opener_Interface::Opener_RemoveExplicitConnection(CipUdint handle)
{
    auto it = Explicit_Connection_set.find(handle);
    if (it != Explicit_Connection_set.end())
    {
        delete Explicit_Connection_set[it];
        Explicit_Connection_set.erase (it);
    }
    return;
}

//Open a new IO connection
CipUdint Opener_Interface::Opener_CreateIOConnection()
{
    CipUdint handle = IO_Connection_set.size();
    IO_Connection_set.emplace(handle, new Opener_IOConnection());
    return handle;
}

//Destroy the IO Connection
void Opener_Interface::Opener_RemoveIOConnection(CipUdint handle)
{
    auto it = IO_Connection_set.find(handle);
    if (it != IO_Connection_set.end())
    {
        delete IO_Connection_set[it];
        IO_Connection_set.erase (it);
    }
    return;
}

//Get pointer to IO connection
Opener_IOConnection * Opener_Interface::GetOpenerIOConnection(CipUdint handle)
{
    if (IO_Connection_set.find(handle) != IO_Connection_set.end())
        return IO_Connection_set[handle];
    else
        return NULL;
}

//Get pointer to Explicit connection
Opener_ExplicitConnection * Opener_Interface::GetOpenerExplicitConnection(CipUdint handle)
{
    if (Explicit_Connection_set.find(handle) != Explicit_Connection_set.end())
        return Explicit_Connection_set[handle];
    else
        return NULL;
}