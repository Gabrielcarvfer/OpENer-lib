/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

/**
 * @file cipioconnection.h
 * CIP I/O Connection implementation
 * =================================
 *
 *
 * I/O Connection Object State Transition Diagram
 * ----------------------------------------------
 * @dot
 *   digraph IOCIP_ConnectionStateTransition {
 *     A[label="Any State"]
 *     N[label="Non-existent"]
 *     C[label="Configuring"]
 *     E[label="Established"]
 *     W[label="Waiting for Connection ID"]
 *     T[label="Timed Out"]
 *
 *     A->N [label="Delete"]
 *     N->C [label="Create"]
 *     C->C [label="Get/Set/Apply Attribute"]
 *     C->W [label="Apply Attribute"]
 *     W->W [label="Get/Set Attribute"]
 *     C->E [label="Apply Attribute"]
 *     E->E [label="Get/Set/Apply Attribute, Reset, Message Produced/Consumed"]
 *     W->E [label="Apply Attribute"]
 *     E->T [label="Inactivity/Watchdog"]
 *     T->E [label="Reset"]
 *     T->N [label="Delete"]
 *   }
 * @enddot
 *
 */

#ifndef OPENER_CIPIOCONNECTION_H_
#define OPENER_CIPIOCONNECTION_H_


#include "../CIP_Assembly.h"
#include "CIP_Connection.h"
#include "../CIP_CommonPacket.h"
#include "../../Opener_Interface.h"

class CIP_IOConnection : public CIP_Connection
{
    public:
        /** @brief Setup all data in order to establish an IO connection
         *
         * This function can be called after all data has been parsed from the forward open request
         * @param conn_obj pointer to the connection object structure holding the parsed data from the forward open request
         * @param extended_error the extended error code in case an error happened
         * @return general status on the establishment
         *    - EIP_OK ... on success
         *    - On an error the general status code to be put into the response
         */
         CipStatus EstablishIoConnection (CipUint *extended_error);

        /** @brief Take the data given in the connection object structure and open the necessary communication channels
         *
         * This function will use the g_stCPFDataItem!
         * @param conn_obj pointer to the connection object data
         * @return general status on the open process
         *    - EIP_OK ... on success
         *    - On an error the general status code to be put into the response
         */
        CipStatus OpenCommunicationChannels ();

        /** @brief close the communication channels of the given connection and remove it
         * from the active connections list.
         *
         * @param conn_obj pointer to the connection object data
         */
        void CloseCommunicationChannelAndRemoveFromActiveConnectionsList ();

    /**** Global variables ****/
    static CipUsint* g_config_data_buffer; /**< buffers for the config data coming with a forward open request. */
    static unsigned int g_config_data_length;

    static CipUdint g_run_idle_state; /**< buffer for holding the run idle information. */
    static void InitializeIOConnectionData();

private:

        /*The port to be used per default for I/O messages on UDP.*/
        const int kOpenerEipIoUdpPort = 0x08AE;

        /* producing multicast connection have to consider the rules that apply for
         * application connection types.
         */
        CipStatus OpenProducingMulticastConnection(CIP_CommonPacket::PacketFormat* cpf_data);

        CipStatus OpenMulticastConnection(UdpCommuncationDirection direction, CIP_CommonPacket::PacketFormat* cpf_data);

        CipStatus OpenConsumingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data);

        CipStatus OpenProducingPointToPointConnection(CIP_CommonPacket::PacketFormat* cpf_data);

        CipUint HandleConfigData(CIP_Assembly* assembly_class);

        /* Regularly close the IO connection. If it is an exclusive owner or input only
         * connection and in charge of the connection a new owner will be searched
         */
        void CloseIoConnection();

        void HandleIoConnectionTimeOut();

        /** @brief  Send the data from the produced CIP Object of the connection via the socket of the connection object
         *   on UDP.
         *      @param conn_obj  pointer to the connection object
         *      @return status  EIP_OK .. success
         *                     EIP_ERROR .. error
         */
        CipStatus SendConnectedData();

        CipStatus HandleReceivedIoConnectionData(CipUsint* data, CipUint data_length);

};
#endif /* OPENER_CIPIOCONNECTION_H_ */
