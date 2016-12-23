//
// Created by gabriel on 11/11/2016.
//

#include "OpENer_Interface.hpp"
#include "cip/connection/network/NET_NetworkHandler.hpp"
#include "cip/CIP_Common.hpp"

//Initialize static variables
int OpENer_Interface::g_end_stack = 0;
std::map<CipUdint, OpENer_IOConnection*> OpENer_Interface::IO_Connection_set;
std::map<CipUdint, OpENer_ExplicitConnection*> OpENer_Interface::Explicit_Connection_set;

#ifdef USETHREAD
    std::thread * OpENer_Interface::workerThread;
    bool OpENer_Interface::OpENer_active;
#else
    #ifndef WIN32
        #include <unistd.h>
        #include <signal.h>
    #endif
    bool OpENer_Interface::alarmRang = false;
#endif

//Methods
bool OpENer_Interface::OpENer_Initialize()
{
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


#ifdef USETHREAD
        //Create thread to keep OpENer working
        OpENer_active = true;
        workerThread = new std::thread(OpENerWorker);
#else
    #ifndef WIN32
        // register for closing signals so that we can trigger the stack to end
        signal(SIGALRM, alarmRinging);
    #endif
#endif

        return true;
    }


    return false;
}

bool OpENer_Interface::OpENer_Shutdown()
{
    //TODO: clean up all Explicit and IO connections

    //TODO: if cipstatusok, finish handler
    // clean up network state
    //NetworkHandlerFinish ();

    // close remaining sessions and connections, cleanup used data
    //ShutdownCipStack();
    return true;
}

//Thread/Function that makes OpENer work
void OpENer_Interface::OpENerWorker()
{
    unsigned smallerInterval = 1000; // Start as 1s
#ifdef USETHREAD
    while(OpENer_active)
    {
#endif

        //Check every pending connection and timer, send and receive data
        if (kCipStatusOk != NET_NetworkHandler::NetworkHandlerProcessOnce ())
        {
#ifdef USETHREAD
            break;
#endif
        }


#ifdef USETHREAD
        //Set sleep and then loop
        std::this_thread::sleep_for(std::chrono::milliseconds(smallerInterval));
    }
#else
    //Set a alarm to the smaller interval of refresh that connections are configured, so that we don't loose data
    alarmRang = false;
    #ifdef WIN32
        timeSetEvent(smallerInterval, smallerInterval, alarmRinging, NULL, TIME_ONESHOT);
    #else
        alarm(smallerInterval);
    #endif
#endif
}

#ifndef USETHREAD
void OpENer_Interface::alarmRinging()
{

    alarmRang = true;

}
#endif

//Open a new Explicit connection
CipUdint OpENer_Interface::OpENer_CreateExplicitConnection()
{
    CipUdint handle = (CipUdint) Explicit_Connection_set.size();
    //Explicit_Connection_set.emplace(handle, new OpENer_ExplicitConnection());
    return handle;
}

//Destroy Explicit connection and then close it, not in this order
void OpENer_Interface::OpENer_RemoveExplicitConnection(CipUdint handle)
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
CipUdint OpENer_Interface::OpENer_CreateIOConnection()
{
    CipUdint handle = (CipUdint) IO_Connection_set.size();
    //IO_Connection_set.emplace(handle, new OpENer_IOConnection());
    return handle;
}

//Destroy the IO Connection
void OpENer_Interface::OpENer_RemoveIOConnection(CipUdint handle)
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
OpENer_IOConnection * OpENer_Interface::GetOpENerIOConnection(CipUdint handle)
{
    if (IO_Connection_set.find(handle) != IO_Connection_set.end())
        return IO_Connection_set[handle];
    else
        return NULL;
}

//Get pointer to Explicit connection
OpENer_ExplicitConnection * OpENer_Interface::GetOpENerExplicitConnection(CipUdint handle)
{
    if (Explicit_Connection_set.find(handle) != Explicit_Connection_set.end())
        return Explicit_Connection_set[handle];
    else
        return NULL;
}

CipStatus OpENer_Interface::ResetDevice()
{

}

CipStatus OpENer_Interface::ResetDeviceToInitialConfiguration()
{

}

CipStatus OpENer_Interface::AfterDataReceived(void *)
{

}

CipStatus OpENer_Interface::ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway_address)
{

}

void OpENer_Interface::ConfigureMacAddress(const CipUsint* mac_address)
{

}

void OpENer_Interface::ConfigureDomainName(const char* domain_name)
{

}

void OpENer_Interface::ConfigureHostName(const char* host_name)
{

}

void OpENer_Interface::SetDeviceSerialNumber(CipUdint serial_number)
{

}

void OpENer_Interface::SetDeviceStatus(CipUint device_status)
{

}

void OpENer_Interface::CipStackInit(CipUint unique_connection_id)
{
    CIP_Common::CipStackInit (unique_connection_id);
}

void OpENer_Interface::ShutdownCipStack(void)
{
    CIP_Common::ShutdownCipStack ();
}
