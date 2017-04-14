/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIP_CONNECTION_H_
#define OPENER_CIP_CONNECTION_H_


#include <map>
#include <cip/CIP_Objects/CIP_0005_Connection/CIP_Connection.hpp>
#include <cip/CIP_ElectronicKey.hpp>
#include <cip/CIP_Segment.hpp>
#include "../../ciptypes.hpp"
#include "../../connection/CIP_CommonPacket.hpp"
#include "../template/CIP_Object.hpp"
#include "../../connection/network/NET_Connection.hpp"
#include "../../../OpENer_Interface.hpp"
#include "../CIP_0004_Assembly/CIP_Assembly.hpp"

class CIP_ConnectionManager;

typedef CipStatus (*OpenConnectionFunction)(CIP_ConnectionManager* connection, CipUint* extended_error_code);
typedef void (*ConnectionCloseFunction)(CIP_ConnectionManager* connection);

typedef struct {
    CipUdint class_id;
    OpenConnectionFunction open_connection_function;
} ConnectionManagementHandling;



class CIP_ConnectionManager :   public CIP_Object<CIP_ConnectionManager>
{

public:
    static CipStatus Init();
    static CipStatus Shut();

    CipMessageRouterRequest_t  g_message_router_request;
    CipMessageRouterResponse_t g_message_router_response;

    /**
 * @brief Sets the routing type of a connection, either
 * - Point-to-point connections (unicast)
 * - Multicast connection
 */
    typedef enum
    {
        kRoutingTypePointToPointConnection = 0x4000,
        kRoutingTypeMulticastConnection = 0x2000
    } RoutingType;

/** @brief Connection Manager Error codes */
    typedef enum
    {
        kConnectionManagerStatusCodeSuccess = 0x00,
        kConnectionManagerStatusCodeErrorConnectionInUse = 0x0100,
        kConnectionManagerStatusCodeErrorTransportTriggerNotSupported = 0x0103,
        kConnectionManagerStatusCodeErrorOwnershipConflict = 0x0106,
        kConnectionManagerStatusCodeErrorConnectionNotFoundAtTargetApplication = 0x0107,
        kConnectionManagerStatusCodeErrorInvalidOToTConnectionType = 0x123,
        kConnectionManagerStatusCodeErrorInvalidTToOConnectionType = 0x124,
        kConnectionManagerStatusCodeErrorInvalidOToTConnectionSize = 0x127,
        kConnectionManagerStatusCodeErrorInvalidTToOConnectionSize = 0x128,
        kConnectionManagerStatusCodeErrorNoMoreConnectionsAvailable = 0x0113,
        kConnectionManagerStatusCodeErrorVendorIdOrProductcodeError = 0x0114,
        kConnectionManagerStatusCodeErrorDeviceTypeError = 0x0115,
        kConnectionManagerStatusCodeErrorRevisionMismatch = 0x0116,
        kConnectionManagerStatusCodeInvalidConfigurationApplicationPath = 0x0129,
        kConnectionManagerStatusCodeInvalidConsumingApllicationPath = 0x012A,
        kConnectionManagerStatusCodeInvalidProducingApplicationPath = 0x012B,
        kConnectionManagerStatusCodeInconsistentApplicationPathCombo = 0x012F,
        kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened = 0x0119,
        kConnectionManagerStatusCodeErrorParameterErrorInUnconnectedSendService = 0x0205,
        kConnectionManagerStatusCodeErrorInvalidSegmentTypeInPath = 0x0315,
        kConnectionManagerStatusCodeTargetObjectOutOfConnections = 0x011A
    } ConnectionManagerStatusCode_e;

    typedef enum
    {
        kConnMgrServiceGetAttributeAll      = 0x01,
        kConnMgrServiceSetAttributeAll      = 0x02,
        kConnMgrServiceGetAttributeSingle   = 0x0E,
        kConnMgrServiceSetAttributeSingle   = 0x10,
        kConnMgrServiceUnconnectedSend      = 0x52,
        kConnMgrServiceForwardOpen          = 0x54,
        kConnMgrServiceGetConnectionData    = 0x56,
        kConnMgrServiceSearchConnectionData = 0x57,
        kConnMgrServiceObsolete             = 0x59,
        kConnMgrServiceGetConnectionOwner   = 0x5A,
        kConnMgrServiceLargeForwardOpen     = 0x5B,
    } ConnectionManagerServiceCodes_e;



/** @brief macros for comparing sequence numbers according to CIP spec vol
 * 2 3-4.2 for int type variables
 * @define SEQ_LEQ32(a, b) Checks if sequence number a is less or equal than b
 * @define SEQ_GEQ32(a, b) Checks if sequence number a is greater or equal than
 *  b
 *  @define SEQ_GT32(a, b) Checks if sequence number a is greater than b
 */
#define SEQ_LEQ32(a, b) ((int)((a) - (b)) <= 0)
#define SEQ_GEQ32(a, b) ((int)((a) - (b)) >= 0)
#define SEQ_GT32(a, b) ((int)((a) - (b)) > 0)

/** @brief similar macros for comparing 16 bit sequence numbers
 * @define SEQ_LEQ16(a, b) Checks if sequence number a is less or equal than b
 * @define SEQ_GEQ16(a, b) Checks if sequence number a is greater or equal than
 *  b
 */
#define SEQ_LEQ16(a, b) ((short)((a) - (b)) <= 0)
#define SEQ_GEQ16(a, b) ((short)((a) - (b)) >= 0)


/*TEMPORARY EXPLICIT CONNECTION STUFF*/
public:
    static CipStatus EstablishExplicitConnection();
    /*TEMPORARY IO CONNECTION STUFF*/
    /** @brief Setup all data in order to establish an IO connection
         *
         * This function can be called after all data has been parsed from the forward open request
         * @param conn_obj pointer to the connection object structure holding the parsed data from the forward open request
         * @param extended_error the extended error code in case an error happened
         * @return general status on the establishment
         *    - EIP_OK ... on success
         *    - On an error the general status code to be put into the response
         */
    static CipStatus EstablishIoConnection (CIP_Connection *connection, CipUint *extended_error);

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
    static const int kOpENerEipIoUdpPort;

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
public:
    /*END OF TEMPORARY IO CONN STUFF*/

    /** @brief Get a connected object dependent on requested ConnectionID.
     *
     *   @param connection_id  requested @var connection_id of opened connection
     *   @return pointer to connected Object
     *           0 .. connection not present in device
     */
    static CIP_Connection* GetConnectedObject(CipUdint connection_id);
    static CIP_ConnectionManager* GetConnectionManagerObject(CipUdint connection_id);

    /**  Get a connection object for a given output assembly.
     *
     *   @param output_assembly_id requested output assembly of requested
     *connection
     *   @return pointer to connected Object
     *           0 .. connection not present in device
     */
    static CIP_ConnectionManager* GetConnectedOutputAssembly(CipUdint output_assembly_id);

    // Copy the given connection data from pa_pstSrc to pa_pstDst
    static void CopyConnectionData(CIP_Connection* destination, const CIP_Connection* source);

    /** @brief Close the given connection
     *
     * This function will take the data form the connection and correctly closes the
     *connection (e.g., open sockets)
     * @param connection_object pointer to the connection object structure to be
     *closed
     */
    static void CloseConnection (CIP_ConnectionManager * connection_manager_instance);

    /* TODO: Missing documentation */
    static CipBool IsConnectedOutputAssembly(CipUdint instance_number);

    /** @brief Generate the ConnectionIDs and set the general configuration
     * parameter in the given connection object.
     *
     * @param connection_object pointer to the connection object that should be set
     *up.
     */
    static void GeneralConnectionConfiguration(CIP_ConnectionManager * connection_manager_instance);
    static void GeneralConnectionConfiguration(CIP_Connection * connection_instance);

    /** @brief Insert the given connection object to the list of currently active
     *  and managed connections.
     *
     * By adding a connection to the active connection list the connection manager
     * will perform the supervision and handle the timing (e.g., timeout,
     * production inhibit, etc).
     *
     * @param connection_object pointer to the connection object to be added.
     */
   static void AddNewActiveConnection(CIP_Connection * connection_object);

    /* TODO: Missing documentation */
    static void RemoveFromActiveConnections(CIP_ConnectionManager * connection_object);


    static std::map<CipUdint, const CIP_ConnectionManager *> * active_connections_set;

    /** List holding information on the object classes and open/close function
     * pointers to which connections may be established.
     */
    static std::map<CipUdint, ConnectionManagementHandling> *g_astConnMgmList;

    /** buffer connection object needed for forward open */
    static CIP_ConnectionManager *g_dummy_connection_object;

    /** @brief Holds the connection ID's "incarnation ID" in the upper 16 bits */
    static CipUdint g_incarnation_id;

    /* Beginning of instance info */
    /** The data needed for handling connections. This data is strongly related to
     * the connection object defined in the CIP-specification. However the full
     * functionality of the connection object is not implemented. Therefore this
     * data can not be accessed with CIP means.
     */

    typedef struct
    {
        CipUint NumconnEntries;
        CipBool ConnOpenBits[32];//todo: get size from connection max_instances
    }connection_entry_list_t;

    //CIP Attributes
    CipUint Open_requests;
    CipUint Open_format_rejects;
    CipUint Open_resource_rejects;
    CipUint Open_other_rejects;
    CipUint Close_requests;
    CipUint Close_format_requests;
    CipUint Close_other_requests;
    CipUint Connection_timeouts;
    connection_entry_list_t connection_entry_list;
    //attribute 10 is obsolete
    CipUint CPU_utilization;
    CipUdint Max_buff_size;
    CipUdint Buff_size_remaining;

    /* non CIP Attributes, only relevant for opened connections */
    CipByte priority_timetick;
    CipUsint timeout_ticks;
    CipUint connection_serial_number;
    CipUint originator_vendor_id;
    CipUdint originator_serial_number;
    CipUint connection_timeout_multiplier;
    CipUdint o_to_t_requested_packet_interval;
    CipUint o_to_t_network_connection_parameter;
    CipUdint t_to_o_requested_packet_interval;
    CipUint t_to_o_network_connection_parameter;
    CipUsint connection_path_size;
    CIP_Segment electronic_key;
    CipConnectionPath connection_path; // padded EPATH
    //todo: check with connection LinkObject link_object;

    CIP_Connection * consuming_instance;
    /*S_CIP_CM_Object *p_stConsumingCMObject; */

    CIP_Connection * producing_instance;
    /*S_CIP_CM_Object *p_stProducingCMObject; */

    /* the EIP level sequence Count for Class 0/1 Producing Connections may have a different
   value than SequenceCountProducing */
    CipUdint eip_level_sequence_count_producing;

    /* the EIP level sequence Count for Class 0/1 Producing Connections may have a
       different value than SequenceCountProducing */
    CipUdint eip_level_sequence_count_consuming;

    // sequence Count for Class 1 Producing Connections
    CipUint sequence_count_producing;

    // sequence Count for Class 1 Producing Connections
    CipUint sequence_count_consuming;

    CipDint transmission_trigger_timer;
    CipDint inactivity_watchdog_timer;

    /** @brief Minimal time between the production of two application triggered
   * or change of state triggered I/O connection messages
   */
    CipUint production_inhibit_time;

    /** @brief Timer for the production inhibition of application triggered or
   * change-of-state I/O connections.
   */
    CipDint production_inhibit_timer;



    CipUint correct_originator_to_target_size;
    CipUint correct_target_to_originator_size;


    /* public functions */
public:
    CipStatus ForwardClose(CipMessageRouterRequest_t* message_router_request,
                           CipMessageRouterResponse_t* message_router_response);

    CipStatus UnconnectedSend(CipMessageRouterRequest_t* message_router_request,
                              CipMessageRouterResponse_t* message_router_response);

    CipStatus ForwardOpen(CipMessageRouterRequest_t* message_router_request,
                          CipMessageRouterResponse_t* message_router_response);

    CipStatus GetConnectionData(CipMessageRouterRequest_t* message_router_request,
                                CipMessageRouterResponse_t* message_router_response);

    CipStatus SearchConnectionData(CipMessageRouterRequest_t* message_router_request,
                                   CipMessageRouterResponse_t* message_router_response);

    CipStatus GetConnectionOwner(CipMessageRouterRequest_t* message_router_request,
                                 CipMessageRouterResponse_t* message_router_response);

    CipStatus LargeForwardOpen(CipMessageRouterRequest_t* message_router_request,
                               CipMessageRouterResponse_t* message_router_response);

private:
    CipStatus AssembleForwardOpenResponse(CIP_Connection * connection_object, CipMessageRouterResponse_t* message_router_response, CipUsint general_status, CipUint extended_status);

    CipStatus AssembleForwardCloseResponse(
            CipUint connection_serial_number, CipUint originatior_vendor_id,
            CipUdint originator_serial_number,
            CipMessageRouterRequest_t* message_router_request,
            CipMessageRouterResponse_t* message_router_response,
            CipUint extended_error_code);

    /** @brief check if the data given in the connection object match with an already established connection
     *
     * The comparison is done according to the definitions in the CIP specification Section 3-5.5.2:
     * The following elements have to be equal: Vendor ID, Connection Serial Number, Originator Serial Number
     * @param connection_object connection object containing the comparison elements from the forward open request
     * @return
     *    - nullptr if no equal established connection exists
     *    - pointer to the equal connection object
     */
    CIP_ConnectionManager* CheckForExistingConnection(CIP_ConnectionManager* connection_manager_instance);

    /** @brief Compare the electronic key received with a forward open request with the device's data.
     *
     * @param key_format format identifier given in the forward open request
     * @param key_data pointer to the electronic key data received in the forward open request
     * @param extended_status the extended error code in case an error happened
     * @return general status on the establishment
     *    - EIP_OK ... on success
     *    - On an error the general status code to be put into the response
     */
    CipStatus CheckElectronicKeyData(CipUsint key_format, CIP_ElectronicKey * key_data, CipUint* extended_status);

    /** @brief Parse the connection path of a forward open request
     *
     * This function will take the connection object and the received data stream and parse the connection path.
     * @param message_router_request pointer to the received request structure. The position of the data stream pointer has to be at the connection length entry
     * @param extended_status the extended error code in case an error happened
     * @return general status on the establishment
     *    - EIP_OK ... on success
     *    - On an error the general status code to be put into the response
     */
    CipUsint ParseConnectionPath( CipMessageRouterRequest_t* message_router_request, CipUint* extended_error);

    ConnectionManagementHandling* GetConnMgmEntry(CipUdint class_id);


    void AddNullAddressItem(CIP_CommonPacket::PacketFormat* common_data_packet_format_data);

    static CipStatus AddConnectableObject(CipUdint pa_nClassId, OpenConnectionFunction pa_pfOpenFunc)
    {
        return kCipStatusOk;
    }

    static unsigned int GetPaddedLogicalPath(unsigned char* logical_path_segment);

    /** @ingroup CIP_API
      * @brief Trigger the production of an application triggered connection.
      *
      * This will issue the production of the specified connection at the next
      * possible occasion. Depending on the values for the RPI and the production
      * inhibit timer. The application is informed via the
      * EIP_BOOL8 BeforeAssemblyDataSend(S_CIP_Instance *pa_pstInstance)
      * callback function when the production will happen. This function should only
      * be invoked from void HandleApplication(void).
      *
      * The connection can only be triggered if the application is established and it
      * is of application application triggered type.
      *
      * @param output_assembly_id the output assembly connection point of the
      * connection
      * @param input_assembly_id the input assembly connection point of the
      * connection
      * @return EIP_OK on success
      */
    CipStatus TriggerConnections(CipUdint pa_unOutputAssembly, CipUdint pa_unInputAssembly);

    /** @ingroup CIP_API
     * @brief Check if any of the connection timers (TransmissionTrigger or
     * WatchdogTimeout) have timed out.
     *
     * If the a timeout occurs the function performs the necessary action. This
     * function should be called periodically once every OPENER_TIMER_TICK
     * milliseconds.
     *
     * @return EIP_OK on success
     */
    static CipStatus ManageConnections (MilliSeconds elapsed_time);

    static CipUdint GetConnectionId (void);

    /** @ingroup CIP_API
 *  @brief Notify the connection manager that data for a connection has been
 *  received.
 *
 *  This function should be invoked by the network layer.
 *  @param received_data pointer to the buffer of data that has been received
 *  @param received_data_length number of bytes in the data buffer
 *  @param from_address address from which the data has been received. Only
 *           data from the connections originator may be accepted. Avoids
 *           connection hijacking
 *  @return EIP_OK on success
 */
    static CipStatus HandleReceivedConnectedData(CIP_ConnectionManager * connection_manager_instance,
                                                 CipUsint* received_data,
                                                 int data_length,
                                                 struct sockaddr_in* from_address
                                            );


    /* End of instance info */

};

//CipStatus (*OpenConnectionPtr) (CIP_ConnectionManager* connection_object, CipUint* extended_error);

#endif // OPENER_CIP_CONNECTION_H
