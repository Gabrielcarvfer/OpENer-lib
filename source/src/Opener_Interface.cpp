//
// Created by gabriel on 11/11/2016.
//

#include "Opener_Interface.hpp"
#include "cip/connection_stack/network_stack/NET_NetworkHandler.hpp"
#include "cip/CIP_Common.hpp"

bool Opener_Interface::Opener_Initialize()
{
    IO_Connection_set.clear();
    Explicit_Connection_set.clear();
    
    g_end_stack = 0;
    CipUint unique_connection_id;

    //for a real device the serial number should be unique per device
    SetDeviceSerialNumber(123456789);

    // nUniqueConnectionID should be sufficiently random or incremented and stored
    //  in non-volatile memory each time the device boots.

    unique_connection_id = 12321;//rand();

    // Setup the CIP Layer
    CipStackInit(unique_connection_id);

    // Setup Network Handles
    if (kCipStatusOk == NET_NetworkHandler::NetworkHandlerInitialize ())
    {
        g_end_stack = 0;
#ifndef WIN32
        // register for closing signals so that we can trigger the stack to end
        signal(SIGHUP, Opener_Shutdown);
#endif

        // The event loop. Put other processing you need done continually in here
        while (1 != g_end_stack)
        {
            if (kCipStatusOk != NET_NetworkHandler::NetworkHandlerProcessOnce ())
            {
                break;
            }
        }

        return true;
    }

    return false;
}

bool Opener_Interface::Opener_Shutdown()
{
    //TODO: clean up all Explicit and IO connections

    //TODO: if cipstatusok, finish handler
    // clean up network state
    //NetworkHandlerFinish ();

    // close remaining sessions and connections, cleanup used data
    //ShutdownCipStack();
    return true;
}

//Open a new Explicit connection
CipUdint Opener_Interface::Opener_CreateExplicitConnection()
{
    CipUdint handle = Explicit_Connection_set.size();
    //Explicit_Connection_set.emplace(handle, new Opener_ExplicitConnection());
    return handle;
}

//Destroy Explicit connection and then close it, not in this order
void Opener_Interface::Opener_RemoveExplicitConnection(CipUdint handle)
{
    auto it = Explicit_Connection_set.find(handle);
    if (it != Explicit_Connection_set.end())
    {
        //delete Explicit_Connection_set[it];
        Explicit_Connection_set.erase (it);
    }
    return;
}

//Open a new IO connection
CipUdint Opener_Interface::Opener_CreateIOConnection()
{
    CipUdint handle = IO_Connection_set.size();
    //IO_Connection_set.emplace(handle, new Opener_IOConnection());
    return handle;
}

//Destroy the IO Connection
void Opener_Interface::Opener_RemoveIOConnection(CipUdint handle)
{
    auto it = IO_Connection_set.find(handle);
    if (it != IO_Connection_set.end())
    {
        //delete IO_Connection_set[it];
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

CipStatus Opener_Interface::ResetDevice()
{

}

CipStatus Opener_Interface::ResetDeviceToInitialConfiguration()
{

}

CipStatus Opener_Interface::AfterDataReceived(void *)
{

}

CipStatus Opener_Interface::ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway_address)
{

}

void Opener_Interface::ConfigureMacAddress(const CipUsint* mac_address)
{

}

void Opener_Interface::ConfigureDomainName(const char* domain_name)
{

}

void Opener_Interface::ConfigureHostName(const char* host_name)
{

}

void Opener_Interface::SetDeviceSerialNumber(CipUdint serial_number)
{

}

void Opener_Interface::SetDeviceStatus(CipUint device_status)
{

}

void Opener_Interface::CipStackInit(CipUint unique_connection_id)
{
    CIP_Common::CipStackInit (unique_connection_id);
}

void Opener_Interface::ShutdownCipStack(void)
{
    CIP_Common::ShutdownCipStack ();
}
