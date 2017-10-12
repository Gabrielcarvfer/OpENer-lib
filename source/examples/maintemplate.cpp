#include "OpENer_Interface.hpp"



int main(int argc, char* arg[])
{
    //
    // Before using OpENer, we need to initialize everything and configure it properly
    //
    OpENer_Interface::OpENer_Initialize (12345);

    //TODO: configuration stuff


    //
    // Using an implicit connection on your program
    //
    int imp_conn_handle = OpENer_Interface::OpENer_CreateIOConnection ();

    //TODO: implement IOConnection interface
    //OpENer_IOConnection * imp_conn = OpENer_IOConnection::GetInstance(imp_conn_handle);
    //imp_conn->setInterface();   //the interface determines the interface and associated protocol (EtherNet IP or DeviceNet)
    //imp_conn->setTarget();      //address of receiver
    //imp_conn->setTriggerType(); //polling or signal
    //imp_conn->setMemoryArea();  //where data will be written to be transmitted
    //imp_conn->setTimer();       //only for polling


    //
    // Using an explicit connection on your program
    //
    int exp_conn_handle = OpENer_Interface::OpENer_CreateExplicitConnection ();

    //TODO: implement ExplicitConnection interface
    //OpENer_ExplicitConnection * exp_conn = OpENer_ExplicitConnection::GetInstance(exp_conn_handle);

    //exp_conn->send_message();

    //exp_conn->receive_message();

    //OpENer_Interface::OpENer_RemoveExplicitConnection (exp_conn_handle);



    //Main loop where we produce data to be sent
    bool working = true;
    while (working)
    {

        //If not using a dedicated thread, we need to run networking routines to check every once in a while
        #ifndef USETHREAD
        if(OpENer_Interface::alarmRang)
        {
            OpENer_Interface::OpENerWorker();
        }
        #endif

        //Check if ending command arrived from network
        if (OpENer_Interface::g_end_stack)
            working = false;
    }
    //
    // Before exiting, close all open connections, free memory and finish program
    //
    OpENer_Interface::OpENer_Shutdown();

    return -1;
}

