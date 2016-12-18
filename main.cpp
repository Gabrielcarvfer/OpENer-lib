/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include <Opener_Interface.h>

/******************************************************************************/
int main(int argc, char* arg[])
{
    //
    // Before using OpENer, we need to initialize everything and configure it properly
    //
    Opener_Interface::Opener_Initialize();

    //TODO: configuration stuff


    //
    // Using an implicit connection on your program
    //
    int imp_conn_handle = Opener_Interface::Opener_CreateIOConnection ();

    //TODO: implement IOConnection interface
    //Opener_IOConnection * imp_conn = Opener_IOConnection::GetInstance(imp_conn_handle);
    //imp_conn->setInterface();   //the interface determines the interface and associated protocol (EtherNet IP or DeviceNet)
    //imp_conn->setTarget();      //address of receiver
    //imp_conn->setTriggerType(); //polling or signal
    //imp_conn->setMemoryArea();  //where data will be written to be transmitted
    //imp_conn->setTimer();       //only for polling


    //
    // Using an explicit connection on your program
    //
    int exp_conn_handle = Opener_Interface::Opener_CreateExplicitConnection ();

    //TODO: implement ExplicitConnection interface
    //Opener_ExplicitConnection * exp_conn = Opener_ExplicitConnection::GetInstance(exp_conn_handle);

    //exp_conn->send_message();

    //exp_conn->receive_message();

    Opener_Interface::Opener_RemoveExplicitConnection (exp_conn_handle);


    //
    // Before exiting, close all open connections, free memory and finish program
    //
    Opener_Interface::Opener_Shutdown();

    return -1;
}

