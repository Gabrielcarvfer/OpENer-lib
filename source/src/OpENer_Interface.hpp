//
// Created by gabriel on 11/11/2016.
//
//

#ifndef OPENER_OPENER_INTERFACE_H
#define OPENER_OPENER_INTERFACE_H

//#define USETHREAD

#include <cassert>
#include <signal.h>
#include "cip/ciperror.hpp"
#include "cip/ciptypes.hpp"
#include "cip/CIP_Objects/template/CIP_Object.hpp"
#include "typedefs.hpp"
#include "cip/CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "OpENer_IOConnection.hpp"
#include "OpENer_ExplicitConnection.hpp"

#ifdef USETHREAD
#include <thread>
#endif



/** @mainpage OpENer - Open Source EtherNet/IP(TM) Communication Stack
 *Documentation
 *
 * EtherNet/IP stack for adapter devices (connection target); supports multiple
 * I/O and explicit connections; includes features and objects required by the
 * CIP specification to enable devices to comply with ODVA's conformance/
 * interoperability tests.
 *
 * @section intro_sec Introduction
 *
 * This is the introduction.
 *
 * @section install_sec Building
 * How to compile, install and run OpENer on a specific platform.
 *
 * @subsection build_req_sec Requirements
 * OpENer has been developed to be highly portable. The default version targets
 * PCs with a POSIX operating system and a BSD-socket network interface. To
 * test this version we recommend a Linux PC or Windows with Cygwin installed.
 *  You will need to have the following installed:
 *   - gcc, make, binutils, etc.
 *
 * for normal building. These should be installed on most Linux installations
 * and are part of the development packages of Cygwin.
 *
 * For the development itself we recommend the use of Eclipse with the CDT
 * plugin. For your convenience OpENer already comes with an Eclipse project
 * file. This allows to just import the OpENer source tree into Eclipse.
 *
 * @subsection compile_pcs_sec Compile for PCs
 *   -# Directly in the shell
 *       -# Go into the bin/pc directory
 *       -# Invoke make
 *       -# For invoking opENer type:\n
 *          ./opENer ipaddress subnetmask gateway domainname hostaddress
 * macaddress\n
 *          e.g., ./opENer 192.168.0.2 255.255.255.0 192.168.0.1 test.com
 * testdevice 00 15 C5 BF D0 87
 *   -# Within Eclipse
 *       -# Import the project
 *       -# Go to the bin/pc folder in the make targets view
 *       -# Choose all from the make targets
 *       -# The resulting executable will be in the directory
 *           ./bin/pc
 *       -# The command line parameters can be set in the run configuration
 * dialog of Eclipse
 *
 * @section further_reading_sec Further Topics
 *   - @ref porting
 *   - @ref extending
 *   - @ref license
 *
 * @page porting Porting OpENer
 * @section gen_config_section General Stack Configuration
 * The general stack properties have to be defined prior to building your
 * production. This is done by providing a file called opENer_user_conf.h. An
 * example file can be found in the src/ports/platform-pc directory. The
 * documentation of the example file for the necessary configuration options:
 * opENer_user_conf.h
 *
 * @copydoc opENer_user_conf.h
 *
 * @section startup_sec Startup Sequence
 * During startup of your EtherNet/IP(TM) device the following steps have to be
 * performed:
 *   -# Configure the network properties:\n
 *       With the following functions the network interface of OpENer is
 *       configured:
 *        - EIP_STATUS ConfigureNetworkInterface(const char *ip_address,
 *        const char *subnet_mask, const char *gateway_address)
 *        - void ConfigureMACAddress(const EIP_UINT8 *mac_address)
 *        - void ConfigureDomainName(const char *domain_name)
 *        - void ConfigureHostName(const char *host_name)
 *        .
 *       Depending on your platform these data can come from a configuration
 *       file or from operating system functions. If these values should be
 *       setable remotely via explicit messages the SetAttributeSingle functions
 *       of the EtherNetLink and the TCPIPInterface object have to be adapted.
 *   -# Set the device's serial number\n
 *      According to the CIP specification a device vendor has to ensure that
 *      each of its devices has a unique 32Bit device id. You can set it with
 *      the function:
 *       - void setDeviceSerialNumber(EIP_UINT32 serial_number)
 *   -# Initialize OpENer: \n
 *      With the function CipStackInit(EIP_UINT16 unique_connection_id) the
 *      internal data structures of opENer are correctly setup. After this
 *      step own CIP objects and Assembly objects instances may be created. For
 *      your convenience we provide the call-back function
 *      ApplicationInitialization. This call back function is called when the
 * stack is ready to receive application specific CIP objects.
 *   -# Create Application Specific CIP Objects:\n
 *      Within the call-back function ApplicationInitialization(void) or
 *      after CipStackInit(void) has finished you may create and configure any
 *      CIP object or Assembly object instances. See the module @ref CIP_API
 *      for available functions. Currently no functions are available to
 *      remove any created objects or instances. This is planned
 *      for future versions.
 *   -# Setup the listening TCP and UDP port:\n
 *      THE ETHERNET/IP SPECIFICATION demands from devices to listen to TCP
 *      connections and UDP datagrams on the port AF12hex for explicit messages.
 *      Therefore before going into normal operation you need to configure your
 *      network library so that TCP and UDP messages on this port will be
 *      received and can be hand over to the Ethernet encapsulation layer.
 *
 * @section normal_op_sec Normal Operation
 * During normal operation the following tasks have to be done by the platform
 * specific code:
 *   - Establish connections requested on TCP port AF12hex
 *   - Receive explicit message data on connected TCP sockets and the UPD socket
 *     for port AF12hex. The received data has to be hand over to Ethernet
 *     encapsulation layer with the functions: \n
 *      int HandleReceivedExplictTCPData(int socket_handle, EIP_UINT8* buffer, int
 * buffer_length, int *number_of_remaining_bytes),\n
 *      int HandleReceivedExplictUDPData(int socket_handle, struct sockaddr_in
 * *from_address, EIP_UINT8* buffer, unsigned int buffer_length, int
 * *number_of_remaining_bytes).\n
 *     Depending if the data has been received from a TCP or from a UDP socket.
 *     As a result of this function a response may have to be sent. The data to
 *     be sent is in the given buffer pa_buf.
 *   - Create UDP sending and receiving sockets for implicit connected
 * messages\n
 *     OpENer will use to call-back function int CreateUdpSocket(
 *     UdpCommuncationDirection connection_direction,
 *     struct sockaddr_in *pa_pstAddr)
 *     for informing the platform specific code that a new connection is
 *     established and new sockets are necessary
 *   - Receive implicit connected data on a receiving UDP socket\n
 *     The received data has to be hand over to the Connection Manager Object
 *     with the function EIP_STATUS HandleReceivedConnectedData(EIP_UINT8
 * *data, int data_length)
 *   - Close UDP and TCP sockets:
 *      -# Requested by OpENer through the call back function: void
 * CloseSocket(int socket_handle)
 *      -# For TCP connection when the peer closed the connection OpENer needs
 *         to be informed to clean up internal data structures. This is done
 *with
 *         the function void CloseSession(int socket_handle).
 *      .
 *   - Cyclically update the connection status:\n
 *     In order that OpENer can determine when to produce new data on
 *     connections or that a connection timed out every @ref OPENER_TIMER_TICK
 * milliseconds the
 *     function EIP_STATUS ManageConnections(void) has to be called.
 *
 * @section callback_funcs_sec Callback Functions
 * In order to make OpENer more platform independent and in order to inform the
 * application on certain state changes and actions within the stack a set of
 * call-back functions is provided. These call-back functions are declared in
 * the file OpENer_Interface.h and have to be implemented by the application specific
 * code. An overview and explanation of OpENer's call-back API may be found in
 * the module @ref CIP_CALLBACK_API.
 *
 * @page extending Extending OpENer
 * OpENer provides an API for adding own CIP objects and instances with
 * specific services and attributes. Therefore OpENer can be easily adapted to
 * support different device profiles and specific CIP objects needed for your
 * device. The functions to be used are:
 *   - S_CIP_Class *CreateCIPClass(EIP_UINT32 class_id, int
 * number_of_class_attributes, EIP_UINT32 class_get_attribute_all_mask, int
 * number_of_class_services, int number_of_instance_attributes, EIP_UINT32
 * instance_get_attribute_all_mask, int number_of_instance_services, int
 * number_of_instances, char *class_name, EIP_UINT16 revision);
 *   - S_CIP_Instance *AddCIPInstances(S_CIP_Class *cip_object, int
 * number_of_instances);
 *   - S_CIP_Instance *AddCIPInstance(S_CIP_Class * CIP_ClassInstance, EIP_UINT32
 * instance_id);
 *   - void InsertAttribute(S_CIP_Instance *instance, EIP_UINT16
 * attribute_number, EIP_UINT8 cip_type, void* data);
 *   - void InsertService(S_CIP_Class *class, EIP_UINT8 service_number,
 * CipServiceFunction service_function, char *service_name);
 *
 * @page license OpENer Open Source License
 * The OpENer Open Source License is an adapted BSD style license. The
 * adaptations include the use of the term EtherNet/IP(TM) and the necessary
 * guarding conditions for using OpENer in own products. For this please look
 * in license text as shown below:
 *
 * @include "license.txt"
 *
 */

class OpENer_Interface
{

    public:
        //Initialize or shutdown OpENer CIP stack
        static bool OpENer_Initialize();

        /******************************************************************************/
        /*!\brief Signal handler function for ending stack execution
         *
         * @param pa_nSig the signal we received
         */
        static bool OpENer_Shutdown();

        //Services provided
        //Explicit connections return an object so that you can send data you want, to whom you want and desired network
        static CipUdint OpENer_CreateExplicitConnection();
        static void OpENer_RemoveExplicitConnection(CipUdint handle);

        //Implicit connections receive a pointer and block size, plus whom you want to send and desired network
        static CipUdint OpENer_CreateIOConnection();
        static void OpENer_RemoveIOConnection(CipUdint handle);

        //Get pointers to use
        static OpENer_IOConnection * GetOpENerIOConnection(CipUdint handle);
        static OpENer_ExplicitConnection * GetOpENerExplicitConnection(CipUdint handle);

        static CipStatus AfterDataReceived(void *);


    //TODO: fix
    /** @brief The number of bytes used for the Ethernet message buffer on
     * the pc port. For different platforms it may makes sense to
     * have more than one buffer.
     *
     *  This buffer size will be used for any received message.
     *  The same buffer is used for the replied explicit message.
     */
    #define PC_OPENER_ETHERNET_BUFFER_SIZE 512


    /**  @defgroup CIP_API OpENer User interface
     * @brief This is the public interface of the OpENer. It provides all function
     * needed to implement an EtherNet/IP enabled slave-device.
     */

    /** @ingroup CIP_API
     * @brief Configure the data of the network interface of the device
     *
     *  This function setup the data of the network interface needed by OpENer.
     *  The multicast address is automatically calculated from he given data.
     *
     *  @param ip_address    the current IP address of the device
     *  @param subnet_mask  the subnet mask to be used
     *  @param gateway_address     the gateway address
     *  @return EIP_OK if the configuring worked otherwise EIP_ERROR
     */
    static CipStatus ConfigureNetworkInterface(const char* ip_address, const char* subnet_mask, const char* gateway_address);

    /** @ingroup CIP_API
     * @brief Configure the MAC address of the device
     *
     *  @param mac_address  the hardware MAC address of the network interface
     */
    static void ConfigureMacAddress(const CipUsint* mac_address);

    /** @ingroup CIP_API
     * @brief Configure the domain name of the device
     * @param domain_name the domain name to be used
     */
    static void ConfigureDomainName(const char* domain_name);

    /** @ingroup CIP_API
     * @brief Configure the host name of the device
     * @param host_name the host name to be used
     */
    static void ConfigureHostName(const char* host_name);

    /** @ingroup CIP_API
     * @brief Set the serial number of the device's identity object.
     *
     * @param serial_number unique 32 bit number identifying the device
     */
    static void SetDeviceSerialNumber(CipUdint serial_number);

    /** @ingroup CIP_API
     * @brief Set the current status of the device.
     *
     * @param device_status the new status value
     */
    static void SetDeviceStatus(CipUint device_status);

    /** @ingroup CIP_API
     * @brief Initialize and setup the CIP-stack
     *
     * @param unique_connection_id value passed to Connection_Manager_Init() to form
     * a "per boot" unique connection ID.
     */
    static void CipStackInit(CipUint unique_connection_id);

    /** @ingroup CIP_API
     * @brief Shutdown of the CIP stack
     *
     * This will
     *   - close all open I/O connections,
     *   - close all open explicit connections, and
     *   - free all memory allocated by the stack.
     *
     * Memory allocated by the application will not be freed. This has to be done
     * by the application!
     */
    static void ShutdownCipStack(void);


    /** @ingroup CIP_API
     * @brief Create an instance of an assembly object
     *
     * @param instance_number  instance number of the assembly object to create
     * @param data         pointer to the data the assembly object should contain
     * @param data_length   length of the assembly object's data
     * @return pointer to the instance of the created assembly object. NULL on error
     *
     * Assembly Objects for Configuration Data:
     *
     * The CIP stack treats configuration assembly objects the same way as any other
     * assembly object.
     * In order to support a configuration assembly object it has to be created with
     * this function.
     * The notification on received configuration data is handled with the
     * IApp_after_receive function.
     */
    //static CIP_ClassInstance* CreateAssemblyObject(CipUdint instance_number, CipByte* data, CipUint data_length);

    /** @ingroup CIP_API
     * @brief Function prototype for handling the opening of connections
     *
     * @param CIP_Connection The connection object which is opening the
     * connection
     * @param extended_error_code The returned error code of the connection object
     *
     * @return CIP error code
     */
    //TODO:typedef CipStatus (*OpenConnectionFunction)(CIP_ConnectionManager* connection, CipUint* extended_error_code);

    /** @ingroup CIP_API
     * @brief Function prototype for handling the closing of connections
     *
     * @param CIP_Connection The connection object which is closing the
     * connection
     */
    //TODO:typedef void (*ConnectionCloseFunction)(CIP_ConnectionManager* connection);

    /** @ingroup CIP_API
     * @brief Function prototype for handling the timeout of connections
     *
     * @param CIP_Connection The connection object which connection timed out
     */
    //TODO:typedef void (*ConnectionTimeoutFunction)(CIP_ConnectionManager* connection);

    /** @ingroup CIP_API
     * @brief Function prototype for sending data via a connection
     *
     * @param CIP_Connection The connection object which connection timed out
     *
     * @return EIP stack status
     */
    //TODO:typedef CipStatus (*ConnectionSendDataFunction)(CIP_ConnectionManager* connection);

    /** @ingroup CIP_API
     * @brief Function prototype for receiving data via a connection
     *
     * @param CIP_Connection The connection object which connection timed out
     * @param data The payload of the CIP message
     * @param data_length Length of the payload
     *
     * @return Stack status
     */
    //TODO:typedef CipStatus (*ConnectionReceiveDataFunction)(CIP_ConnectionManager* connection, CipUsint* data, CipUint data_length);

    /** @ingroup CIP_API
     * @brief register open functions for an specific object.
     *
     * With this function any object can be enabled to be a target for forward
     * open/close request.
     * @param class_id The class ID
     * @param open_connection_function   pointer to the function handling the open
     * process
     * @return EIP_OK on success
     */
    //TODO:CipStatus AddConnectableObject(CipUdint class_id, OpenConnectionFunction open_connection_function);





    /** @ingroup CIP_API
     * @brief Inform the encapsulation layer that the remote host has closed the
     * connection.
     *
     * According to the specifications that will clean up and close the session in
     * the encapsulation layer.
     * @param socket_handle the handler to the socket of the closed connection
     */
    static void CloseSession(int socket);

    /**  @defgroup CIP_CALLBACK_API Callback Functions Demanded by OpENer
     * @ingroup CIP_API
     *
     * @brief These functions have to implemented in order to give the OpENer a
     * method to inform the application on certain state changes.
     */

    /** @ingroup CIP_CALLBACK_API
     * @brief Callback for the application initialization
     *
     * This function will be called by the CIP stack after it has finished its
     * initialization. In this function the user can setup all CIP objects she
     * likes to have.
     *
     * This function is provided for convenience reasons. After the void
     * CipStackInit(void)
     * function has finished it is okay to also generate your CIP objects.
     *  return status EIP_ERROR .. error
     *                EIP_OK ... successful finish
     */
    static CipStatus ApplicationInitialization(void);

    /** @ingroup CIP_CALLBACK_API
     * @brief Allow the device specific application to perform its execution
     *
     * This function will be executed by the stack at the beginning of each
     * execution of EIP_STATUS ManageConnections(void). It allows to implement
     * device specific application functions. Execution within this function should
     * be short.
     */
    static void HandleApplication(void);

    /** @ingroup CIP_CALLBACK_API
     * @brief Inform the application on changes occurred for a connection
     *
     * @param output_assembly_id the output assembly connection point of the
     * connection
     * @param input_assembly_id the input assembly connection point of the
     * connection
     * @param io_connection_event information on the change occurred
     */
    static void CheckIoConnectionEvent(unsigned int output_assembly_id, unsigned int input_assembly_id, IoConnectionEvent io_connection_event);

    /** @ingroup CIP_CALLBACK_API
     * @brief Call back function to inform application on received data for an
     * assembly object.
     *
     * This function has to be implemented by the user of the CIP-stack.
     * @param instance pointer to the assembly object data was received for
     * @return Information if the data could be processed
     *     - EIP_OK the received data was ok
     *     - EIP_ERROR the received data was wrong (especially needed for
     * configuration data assembly objects)
     *
     * Assembly Objects for Configuration Data:
     * The CIP-stack uses this function to inform on received configuration data.
     * The length of the data is already checked within the stack. Therefore the
     * user only has to check if the data is valid.
     */
    //CipStatus AfterAssemblyDataReceived(CIP_ClassInstance* instance);

    /** @ingroup CIP_CALLBACK_API
     * @brief Inform the application that the data of an assembly
     * object will be sent.
     *
     * Within this function the user can update the data of the assembly object
     * before it gets sent. The application can inform the application if data has
     * changed.
     * @param instance instance of assembly object that should send data.
     * @return data has changed:
     *          - true assembly data has changed
     *          - false assembly data has not changed
     */
    //static CipBool BeforeAssemblyDataSend(CIP_ClassInstance* instance);

    /** @ingroup CIP_CALLBACK_API
     * @brief Emulate as close a possible a power cycle of the device
     *
     * @return if the service is supported the function will not return.
     *     EIP_ERROR if this service is not supported
     */
    static CipStatus ResetDevice(void);

    /** @ingroup CIP_CALLBACK_API
     * @brief Reset the device to the initial configuration and emulate as close as
     * possible a power cycle of the device
     *
     * @return if the service is supported the function will not return.
     *     EIP_ERROR if this service is not supported
     */
    static CipStatus ResetDeviceToInitialConfiguration(void);

    /** @ingroup CIP_CALLBACK_API
     * @brief Allocate memory for the CIP stack
     *
     * emulate the common c-library function calloc
     * In OpENer allocation only happens on application startup and on
     * class/instance creation and configuration not on during operation
     * (processing messages).
     * @param number_of_elements number of elements to allocate
     * @param size_of_element size in bytes of one element
     * @return pointer to the allocated memory, 0 on error
     */
    //void* CipCalloc(unsigned int number_of_elements, unsigned int size_of_element);

    /** @ingroup CIP_CALLBACK_API
     * @brief Free memory allocated by the OpENer
     *
     * emulate the common c-library function free
     * @param pa_poData pointer to the allocated memory
     */
    //void CipFree(void* data);

    /** @ingroup CIP_CALLBACK_API
     * @brief Inform the application that the Run/Idle State has been changed
     * by the originator.
     *
     * @param run_idle_value the current value of the run/idle flag according to CIP
     * spec Vol 1 3-6.5
     */
    void RunIdleChanged(CipUdint run_idle_value);


    /** @ingroup CIP_CALLBACK_API
     * @brief Close the given socket and clean up the stack
     *
     * @param socket_handle socket descriptor to close
     */
    void CloseSocket(int socket);

    /*! \brief Flag indicating if the stack should end its execution
       */
    static int g_end_stack;

    //Execute everything needed to make library work and then go back to the main program/or run in a thread
    static void OpENerWorker();
#ifndef USETHREAD
    static bool alarmRang;
#endif
    protected:
        static std::map<CipUdint, OpENer_IOConnection*> IO_Connection_set;
        static std::map<CipUdint, OpENer_ExplicitConnection*> Explicit_Connection_set;

#ifdef USETHREAD
        static std::thread *workerThread;
        static bool OpENer_active;
#else
    #ifdef WIN32
        static void alarmRinging(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    #elif __linux__
        static void alarmRinging(int signal);
    #endif
#endif



};


#endif //OPENER_OPENER_INTERFACE_H
