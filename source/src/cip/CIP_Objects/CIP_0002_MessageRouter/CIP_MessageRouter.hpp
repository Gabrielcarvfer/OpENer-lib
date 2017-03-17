/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIPMESSAGEROUTER_H_
#define OPENER_CIPMESSAGEROUTER_H_


#include "../../ciptypes.hpp"
#include "../template/CIP_Object.hpp"
#include <map>


/** @brief Structure for storing the Response generated by an explict message.
 *
 *  This buffer will be used for storing the result. The response message will be generated
 *  by assembleLinearMsg.
 */
class CIP_MessageRouter;
class CIP_MessageRouter : public CIP_Object<CIP_MessageRouter>
{
    public:
        typedef struct {
            CipUint number;
            std::vector<CipUint> classes;
        } object_list_t;

        typedef enum {
           kCipSymbolicPathUnknown = 0, //path not recognized by processing node
           kCipSymbolicPathDestinationNotAssigned = 1, //recognized but not associated
           kCipSymbolicPathSegmentError = 2 //syntax can't be understood by the node
        } CipSymbolicPath_e;

        static CipMessageRouterRequest_t  *g_message_router_request;
        static CipMessageRouterResponse_t *g_message_router_response;

        static std::vector<CipUsint> *g_message_data_reply_buffer;
        /** @brief Initialize the data structures of the message router
         *  @return kCipStatusOk if class was initialized, otherwise kCipStatusError
         */
        static CipStatus Init (void);
        static CipStatus Shut (void);


        /** @brief A class registry
         *
         * A std::map is used as the registry of classes known to the message router
         * for small devices with very limited memory it could make sense to change this list into an
         * array with a given max size for removing the need for having to dynamically allocate
         * memory. The size of the array could be a parameter in the platform config file.
         */
        static std::map<CipUdint, void*> message_router_registered_classes;


        /*! Register a class at the message router.
         *  In order that the message router can deliver
         *  explicit messages each class has to register.
         *  Will be automatically done when invoking create
         *  createCIPClass.
         *  @param object CIP class to be registered
         *  @return EIP_OK on success
         */
        /** @brief Register an Class to the message router
         *  @param CIP_ClassInstance Pointer to a class object to be registered.
         *  @return status      0 .. success
         *                     -1 .. error no memory available to register more objects
         */
        static CipStatus RegisterCIPClass(void * CIP_ClassInstance);


        /** @brief Get the registered MessageRouter object corresponding to ClassID.
         *  given a class ID, return a pointer to the registration node for that object
         *
         *  @param class_id Class code to be searched for.
         *  @return Pointer to registered message router object
         *      0 .. Class not registered
         */
        static void* GetRegisteredObject(CipUdint class_id);


        /** @brief Create Message Router Request structure out of the received data.
         *
         * Parses the UCMM header consisting of: service, IOI size, IOI, data into a request structure
         * @param data pointer to the message data received
         * @param data_length number of bytes in the message
         * @param message_router_request pointer to structure of MRRequest data item.
         * @return status  0 .. success
         *                 -1 .. error
         */
        static CipStatus CreateMessageRouterRequestStructure(CipUsint* data, CipInt data_length, CipMessageRouterRequest_t* message_router_request);


        /** @brief Notify the MessageRouter that an explicit message (connected or unconnected)
         *  has been received. This function will be called from the encapsulation layer.
         *  The CPF structure is already parsed an can be accessed via the global variable:
         *  g_stCPFDataItem.
         *  @param data pointer to the data buffer of the message directly at the beginning of the CIP part.
         *  @param data_length number of bytes in the data buffer
         *  @return  EIP_ERROR on fault
         *           EIP_OK on success
         */
        static CipStatus NotifyMR(CipUsint* data, int data_length);


        /** @brief Free all data allocated by the classes created in the CIP stack
         */
        static void DeleteAllClasses(void);

        //temporary
        static CipStatus notify_application(CipEpath target_epath, CipUint target_epath_size, CipNotification notification);
        static CipStatus route_message( CipMessageRouterRequest_t *request, CipMessageRouterResponse_t *response );

    //CIP attributes
    //Class attributes are only the ones already specified in the template
    //Instance attributes
    object_list_t object_list;
    CipUint number_avaiable;
    CipUint number_active;
    std::vector<CipUint>active_connections;

    //CIP services
    static CipStatus symbolic_translation(CipEpath *symbolic_epath, CipEpath *logical_epath);
    static std::map<CipUdint, void*> *registered_objects;
};
#endif /* OPENER_CIPMESSAGEROUTER_H_ */
