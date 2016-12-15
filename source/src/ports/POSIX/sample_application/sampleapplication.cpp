/*******************************************************************************
 * Copyright (c) 2012, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include "Opener_Interface.h"
#include <stdlib.h>
#include <string.h>

#define DEMO_APP_INPUT_ASSEMBLY_NUM 100 //0x064
#define DEMO_APP_OUTPUT_ASSEMBLY_NUM 150 //0x096
#define DEMO_APP_CONFIG_ASSEMBLY_NUM 151 //0x097
#define DEMO_APP_HEARBEAT_INPUT_ONLY_ASSEMBLY_NUM 152 //0x098
#define DEMO_APP_HEARBEAT_LISTEN_ONLY_ASSEMBLY_NUM 153 //0x099
#define DEMO_APP_EXPLICT_ASSEMBLY_NUM 154 //0x09A

/* global variables for demo application (4 assembly data fields)  ************/

CipUsint g_assembly_data064[32]; /* Input */
CipUsint g_assembly_data096[32]; /* Output */
CipUsint g_assembly_data097[10]; /* Config */
CipUsint g_assembly_data09A[32]; /* Explicit */

CipStatus ApplicationInitialization(void)
{
    /* create 3 assembly object instances*/
    /*INPUT*/
    CreateAssemblyObject(DEMO_APP_INPUT_ASSEMBLY_NUM, &g_assembly_data064[0],
        sizeof(g_assembly_data064));

    /*OUTPUT*/
    CreateAssemblyObject(DEMO_APP_OUTPUT_ASSEMBLY_NUM, &g_assembly_data096[0],
        sizeof(g_assembly_data096));

    /*CONFIG*/
    CreateAssemblyObject(DEMO_APP_CONFIG_ASSEMBLY_NUM, &g_assembly_data097[0],
        sizeof(g_assembly_data097));

    /*Heart-beat output assembly for Input only connections */
    CreateAssemblyObject(DEMO_APP_HEARBEAT_INPUT_ONLY_ASSEMBLY_NUM, 0, 0);

    /*Heart-beat output assembly for Listen only connections */
    CreateAssemblyObject(DEMO_APP_HEARBEAT_LISTEN_ONLY_ASSEMBLY_NUM, 0, 0);

    /* assembly for explicit messaging */
    CreateAssemblyObject(DEMO_APP_EXPLICT_ASSEMBLY_NUM, &g_assembly_data09A[0],
        sizeof(g_assembly_data09A));

    ConfigureExclusiveOwnerConnectionPoint(0, DEMO_APP_OUTPUT_ASSEMBLY_NUM,
        DEMO_APP_INPUT_ASSEMBLY_NUM,
        DEMO_APP_CONFIG_ASSEMBLY_NUM);
    ConfigureInputOnlyConnectionPoint(0,
        DEMO_APP_HEARBEAT_INPUT_ONLY_ASSEMBLY_NUM,
        DEMO_APP_INPUT_ASSEMBLY_NUM,
        DEMO_APP_CONFIG_ASSEMBLY_NUM);
    ConfigureListenOnlyConnectionPoint(0,
        DEMO_APP_HEARBEAT_LISTEN_ONLY_ASSEMBLY_NUM,
        DEMO_APP_INPUT_ASSEMBLY_NUM,
        DEMO_APP_CONFIG_ASSEMBLY_NUM);

    return kCipStatusOk;
}

void HandleApplication(void)
{
    /* check if application needs to trigger an connection */
}

void CheckIoConnectionEvent(unsigned int output_assembly_id,
    unsigned int input_assembly_id,
    IoConnectionEvent io_connection_event)
{
    /* maintain a correct output state according to the connection state*/

    (void)output_assembly_id; /* suppress compiler warning */
    (void)input_assembly_id; /* suppress compiler warning */
    (void)io_connection_event; /* suppress compiler warning */
}

CipStatus AfterAssemblyDataReceived(CIPClass* instance)
{
    CipStatus status = kCipStatusOk;

    /*handle the data received e.g., update outputs of the device */
    switch (instance->instance_number) {
    case DEMO_APP_OUTPUT_ASSEMBLY_NUM:
        /* Data for the output assembly has been received.
       * Mirror it to the inputs */
        memcpy(&g_assembly_data064[0], &g_assembly_data096[0],
            sizeof(g_assembly_data064));
        break;
    case DEMO_APP_EXPLICT_ASSEMBLY_NUM:
        /* do something interesting with the new data from
       * the explicit set-data-attribute message */
        break;
    case DEMO_APP_CONFIG_ASSEMBLY_NUM:
        /* Add here code to handle configuration data and check if it is ok
       * The demo application does not handle config data.
       * However in order to pass the test we accept any data given.
       * EIP_ERROR
       */
        status = kCipStatusOk;
        break;
    }
    return status;
}

CipBool BeforeAssemblyDataSend(CIPClass* pa_pstInstance)
{
    /*update data to be sent e.g., read inputs of the device */
    /*In this sample app we mirror the data from out to inputs on data receive
   * therefore we need nothing to do here. Just return true to inform that
   * the data is new.
   */

    if (pa_pstInstance->instance_number == DEMO_APP_EXPLICT_ASSEMBLY_NUM) {
        /* do something interesting with the existing data
     * for the explicit get-data-attribute message */
    }
    return true;
}

CipStatus ResetDevice(void)
{
    /* add reset code here*/
    return kCipStatusOk;
}

CipStatus ResetDeviceToInitialConfiguration(void)
{
    /*rest the parameters */

    /*than perform device reset*/
    ResetDevice();
    return kCipStatusOk;
}

void* CipCalloc(unsigned int number_of_elements, unsigned size_of_element)
{
    return calloc(number_of_elements, size_of_element);
}

void CipFree(void* data)
{
    free(data);
}

void RunIdleChanged(CipUdint run_idle_value)
{
    (void)run_idle_value;
}