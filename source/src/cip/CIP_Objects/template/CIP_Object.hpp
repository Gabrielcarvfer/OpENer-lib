//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 18/11/2016.
//

#ifndef OPENER_CIP_OBJECT_H
#define OPENER_CIP_OBJECT_H

#include "../../ciptypes.hpp"
#include <map>
#include <string>

#include "CIP_Attribute.hpp"
#include "CIP_Service.hpp"

template <class T>
class CIP_Object
{
    public:
        static const T * GetInstance(CipUdint instance_number);
        static const T * GetClass();
        static CipUdint GetNumberOfInstances();
        static CipDint GetInstanceNumber(const T * instance);
        static bool AddClassInstance(T * instance, CipUdint position);
        static bool RemoveClassInstance(T * instance);
        static bool RemoveClassInstance(CipUdint position);

        static CipStatus Init();
        static CipStatus Shut();
        CIP_Object();
        virtual ~CIP_Object();

        static CipUdint class_id;
        static std::string class_name;
        static CipUint  revision;
        static CipUint  max_instances;
        static CipUint  number_of_instances;
        static CipUdint optional_attribute_list;
        static CipUdint optional_service_list;
        static CipUint  maximum_id_number_class_attributes;
        static CipUint  maximum_id_number_instance_attributes;

        std::map< CipUdint, CIP_Attribute * > attributes;
        std::map< CipUdint, CIP_Service * > services;


        /** @ingroup CIP_API
        * @brief Allocate memory for new CIP Class and attributes
        *
        *  The new CIP class will be registered at the stack to be able
        *  for receiving explicit messages.
        *
        *  @param class_id class ID of the new class
        *  @param get_attribute_all_mask mask of which attributes are included in the
        *  class getAttributeAll.
        *  If the mask is 0 the getAttributeAll service will not be added as class
        *  service
        *  @param instance_attributes_get_attributes_all_mask  mask of which attributes
        *  are included in the instance getAttributeAll
        *  If the mask is 0 the getAttributeAll service will not be added as class
        *  service
        *  @param class_name  class name (for debugging class structure)
        *  @param class_revision class revision
        *  @return pointer to new class object
        *      0 on error
        */

/** @ingroup CIP_API
               * @brief Get a pointer to an instance
               *
               * @param cip_object pointer to the object the instance belongs to
               * @param instance_number number of the instance to retrieve
               * @return pointer to CIP Instance
               *          0 if instance is not in the object
               */


    /** @ingroup CIP_API
     * @brief Insert an attribute in an instance of a CIP class
     *
     *  Note that attributes are stored in an array pointer in the instance
     *  the attributes array is not expandable if you insert an attributes that has
     *  already been defined, the previous attributes will be replaced
     *
     *  @param pa_pInstance pointer to CIP class. (may be also instance 0)
     *  @param pa_nAttributeNr number of attribute to be inserted.
     *  @param cip_data_type type of attribute to be inserted.
     *  @param cip_data pointer to data of attribute.
     *  @param cip_flags flags to indicate set-ability and get-ability of attribute.
     */
    void InsertAttribute(CipUint attribute_number, CipUsint cip_type, void* data, CipAttributeFlag cip_flags);


    /** @ingroup CIP_API
    * @brief Insert a service in an instance of a CIP object
    *
    *  Note that services are stored in an array pointer in the class object
    *  the service array is not expandable if you insert a service that has
    *  already been defined, the previous service will be replaced
    *
    * @param cip_class_to_add_service pointer to CIP object. (may be also
    * instance# 0)
    * @param service_code service code of service to be inserted.
    * @param service_function pointer to function which represents the service.
    * @param service_name name of the service
    */
    //void InsertService(CipUsint service_number, CIP_Service service_function, std::string service_name);


    /** @ingroup CIP_API
     * @brief Get a pointer to an instance's attribute
     *
     * As instances and objects are selfsimilar this function can also be used
     * to retrieve the attribute of an object.
     * @param cip_instance  pointer to the instance the attribute belongs to
     * @param attribute_number number of the attribute to retrieve
     * @return pointer to attribute
     *          0 if instance is not in the object
     */
    CIP_Attribute* GetCipAttribute(CipUint attribute_number);


    /** @brief Generic implementation of the GetAttributeAll CIP service
     *
     * Copy all attributes from Object into the global message buffer.
     * @param instance pointer to object instance with data.
     * @param message_router_request pointer to MR request.
     * @param message_router_response pointer for MR response.
     * @return length of data stream >0 .. success
     *              0 .. no reply to send
     */
    CipStatus GetAttributeAll(CipMessageRouterRequest_t * message_router_request,CipMessageRouterResponse_t* message_router_response);


    /** @brief Generic implementation of the GetAttributeSingle CIP service
     *
     *  Check from classID which Object requests an attribute, search if object has
     *  the appropriate attribute implemented.
     * @param instance pointer to instance.
     * @param message_router_request pointer to request.
     * @param message_router_response pointer to response.
     * @return status  >0 .. success
     *          -1 .. requested attribute not available
     */
    CipStatus GetAttributeSingle(CipMessageRouterRequest_t * message_router_request,CipMessageRouterResponse_t* message_router_response);
    /* type definition of CIP service structure */

    virtual CipStatus InstanceServices(int service, CipMessageRouterRequest_t * msg_router_request,CipMessageRouterResponse_t* msg_router_response)=0;

    protected:
        //Class stuff
        static T * class_ptr;
        static std::map<CipUdint, const T *> object_Set;

        //Instance stuff
        int id;
};

#include "CIP_Object_impl.hpp"
#endif //OPENER_CIP_OBJECT_H
