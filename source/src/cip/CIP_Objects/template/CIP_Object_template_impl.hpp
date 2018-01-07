//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 18/11/2016.
// (code fom GetAttribute functions originally from OpENer)
//
#ifndef CIP_OBJECT_IMPL_H
#define CIP_OBJECT_IMPL_H

#include <trace.hpp>
//#include <CIP_Common.hpp>
#include <opener_user_conf.hpp>
#include <utility>
#include <ciptypes.hpp>
#include <ciptypes.hpp>

//Static variables
template<class T> CipUint    CIP_Object_template<T>::class_id = 0;
template<class T> std::string CIP_Object_template<T>::class_name = "";
template<class T> CipUint     CIP_Object_template<T>::revision = 0;
template<class T> CipUint     CIP_Object_template<T>::max_instances = 1;
template<class T> CipUint     CIP_Object_template<T>::number_of_instances = 0;
template<class T> CipUdint    CIP_Object_template<T>::optional_attribute_list = 0;
template<class T> CipUdint    CIP_Object_template<T>::optional_service_list = 0;
template<class T> CipUint     CIP_Object_template<T>::maximum_id_number_class_attributes = 0;
template<class T> CipUint     CIP_Object_template<T>::maximum_id_number_instance_attributes = 0;
template<class T> std::map<CipUdint, const T *> CIP_Object_template<T>::object_Set;

template<class T> std::map<CipUsint, CipAttrInfo_t> CIP_Object_template<T>::classAttrInfo;
template<class T> std::map<CipUsint, CipServiceProperties_t>   CIP_Object_template<T>::classServicesProperties;
template<class T> std::map<CipUsint, CipAttrInfo_t> CIP_Object_template<T>::instAttrInfo;
template<class T> std::map<CipUsint, CipServiceProperties_t>   CIP_Object_template<T>::instanceServicesProperties;

//Methods
template <class T>
CIP_Object_template<T>::CIP_Object_template()
{
    if (number_of_instances < max_instances)
    {
        id = number_of_instances;
        classId = class_id;
        number_of_instances++;
    }
    else
    {
        throw std::range_error("Requested instance exceeds maximum specified to " + class_name);
    }
}

template <class T>
CIP_Object_template<T>::~CIP_Object_template()
{

}

template <class T>
const T * CIP_Object_template<T>::GetInstance(CipUdint instance_number)
{
    if (object_Set.size() >= instance_number)
        return object_Set[instance_number];
    else
        return nullptr;
}

template <class T>
const T  * CIP_Object_template<T>::GetClass()
{
    return GetInstance(0);
}

template <class T>
CipUdint CIP_Object_template<T>::GetNumberOfInstances()
{
    return (CipUdint)object_Set.size();
}

template <class T>
CipDint CIP_Object_template<T>::GetInstanceNumber(const T  * instance)
{
    for (auto it = object_Set.begin(); it != object_Set.end(); it++)
    {
        if (it->second == instance)
        {
            object_Set.erase(it);
            return it->first;
        }
    }
    return -1;
}

template <class T>
bool CIP_Object_template<T>::AddClassInstance(T  * instance, CipDint position)
{
    if (instance == nullptr)
    {
        return false;
    }
    //If passed position is -1, then search first free position
    if (position == -1)
    {
		unsigned i;
        for ( i = 1; i < object_Set.size(); i++)
        {
            auto it = object_Set.find(i);
            if (it == object_Set.end())
                break;
        }
        position = i;
    }

    //Emplace instance
    object_Set.emplace(position,instance);

    //Check if instance was added correctly
    auto it = object_Set.find(position);
    return (it != object_Set.end());
}

template <class T>
bool CIP_Object_template<T>::RemoveClassInstance(T  * instance)
{
    for (auto it = object_Set.begin(); it != object_Set.end(); it++)
    {
        if (it->second == instance)
        {
            object_Set.erase(it);
            return true;
        }
    }
    return false;
}

template <class T>
bool CIP_Object_template<T>::RemoveClassInstance(CipUdint position)
{
    if ( object_Set.find(position) != object_Set.end() )
    {
        object_Set.erase (position);
        return true;
    }
    else
    {
        return false;
    }
}

template <class T>
CIP_Attribute CIP_Object_template<T>::GetCipAttribute(CipUsint attribute_number)
{
	bool isClass = this->id == 0;
    CIP_Attribute attr{kCipAny, nullptr};

    //todo: check if attribute is Gettable here or outside? If here, we can return a nullptr instead of the memory
    //  so that we can identify that, although attribute exists (by it's type), it's read protected
	if (isClass)
    {
        //If class instance, check if attribute exists
        if (classAttrInfo.find(attribute_number) != classAttrInfo.end())
        {
            //If attribute exists, then return an attribute containing attribute content type and pointer to it
            attr.type_id = classAttrInfo[attribute_number].attributeType;
            attr.value_ptr.raw_ptr = this->retrieveAttribute(attribute_number);

        }
    }
	else
	{
        //If a common instance, check if attribute exists
        if (instAttrInfo.find(attribute_number) != instAttrInfo.end())
        {
            //If attribute exists, then return an attribute containing attribute content type and pointer to it
            attr.type_id = instAttrInfo[attribute_number].attributeType;
            attr.value_ptr.raw_ptr = this->retrieveAttribute(attribute_number);
        }
	}

    //Log error and return an attribute containing an invalid Cip type and a nullptr
	OPENER_TRACE_WARN("attribute %d not defined\n", attribute_number);
	return attr;
}

template <class T>
CipStatus CIP_Object_template<T>::GetAttributeSingle(CipMessageRouterRequest_t* message_router_request,
                                            CipMessageRouterResponse_t* message_router_response)
{
    // Mask for filtering get-ability
    CipByte get_mask;

    CipUsint attribute_number = message_router_request->request_path.attribute_number;
    CipAttrInfo_t* attribute = &instAttrInfo.at(attribute_number);


    message_router_response->reply_service = (0x80 | message_router_request->service);
    message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSupported;
    message_router_response->size_additional_status = 0;

    // set filter according to service: get_attribute_all or get_attribute_single
    if (kServiceGetAttributeAll == message_router_request->service)
    {
        get_mask = kAttrFlagGetableAll;
        message_router_response->general_status = kCipGeneralStatusCodeSuccess;
    }
    else
    {
        get_mask = kAttrFlagGetableSingle;
    }

    if ((attribute != nullptr) && (retrieveAttribute(message_router_request->request_path.attribute_number) != 0))
    {
        if (attribute->attributeFlag & get_mask)
        {
            OPENER_TRACE_INFO("getAttribute %d\n",
                message_router_request->request_path.attribute_number); // create a reply message containing the data
            //TODO think if it is better to put this code in an own getAssemblyAttributeSingle functions which will call get attribute single.
            

            if (attribute->attributeType == kCipByteArray && this->class_id == kCipAssemblyClassCode)
            {
                // we are getting a byte array of a assembly object, kick out to the app callback
                OPENER_TRACE_INFO(" -> getAttributeSingle CIP_BYTE_ARRAY\r\n");

                //TODO:build an alternative
                //BeforeAssemblyDataSend(this);
            }
            EncodeData(attribute->attributeType, this->retrieveAttribute(attribute_number), &(message_router_response->response_data));
            message_router_response->general_status = kCipGeneralStatusCodeSuccess;
        }
    }

    return CipStatus(kCipGeneralStatusCodeSuccess);
}

template <class T>
CipStatus CIP_Object_template<T>::GetAttributeAll(CipMessageRouterRequest_t* message_router_request,
                                         CipMessageRouterResponse_t* message_router_response)
{
    std::vector<CipUsint> reply;

    CipServiceProperties_t serviceProperties;
    CipAttrInfo_t attributeProperties;

    auto it = instanceServicesProperties.find(kServiceGetAttributeAll);

    if ( it != instanceServicesProperties.end())
    {
        serviceProperties = it->second;
        if (0 == classAttrInfo.size())
        {
            //there are no attributes to be sent back
            message_router_response->reply_service = (0x80 | message_router_request->service);
            message_router_response->general_status = kCipGeneralStatusCodeServiceNotSupported;
            message_router_response->size_additional_status = 0;
        }
        else
        {
            auto attrIt = instAttrInfo.begin ();
            int max_attribute_id = ( this->id == 0 ? this->maximum_id_number_class_attributes : this->maximum_id_number_instance_attributes);
            for (; attrIt != instAttrInfo.end(); attrIt++) // for each instance attribute of this class
            {
                attributeProperties = attrIt->second;
                int attrNum = attrIt->first;

                // only return attributes that are flagged as being part of GetAttributeALl
                if (attrNum < max_attribute_id && (attributeProperties.attributeFlag & kAttrFlagGetableAll))
                {
                    if (kCipGeneralStatusCodeSuccess != this->InstanceServices(kServiceGetAttributeAll, message_router_request, message_router_response).status)
                    {
                        message_router_response->response_data.emplace (message_router_response->response_data.begin (), reply);

                        return CipStatus(kCipStatusError);
                    }
                    //message_router_response->data += message_router_response->data_length;
                }
            }
            //message_router_response->data_length = message_router_response->data - reply;
            message_router_response->response_data.emplace (message_router_response->response_data.begin (), reply);
        }
        return CipStatus(kCipGeneralStatusCodeSuccess);
    }
    return CipStatus(kCipGeneralStatusCodeSuccess); // Return kCipGeneralStatusCodeSuccess if cannot find GET_ATTRIBUTE_SINGLE service
}

template <class T>
CipStatus CIP_Object_template<T>::InstanceServices(int service, CipMessageRouterRequest_t* msg_router_request,
                                          CipMessageRouterResponse_t* msg_router_response)
{
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeServiceNotSupported;
	stat.extended_status = 0;

    //Class services
    if (this->id == 0)
    {
        if (classServicesProperties.find(service) != classServicesProperties.end())
        {
			stat = this->retrieveService(service, msg_router_request, msg_router_response);
        }
    }
    //Instance services
    else
    {
        if (instanceServicesProperties.find(service) != instanceServicesProperties.end())
        {
            stat = this->retrieveService(service, msg_router_request, msg_router_response);
        }
    }
	return stat;
}


template <class T>
CipStatus CIP_Object_template<T>::SetAttributeSingle(CipMessageRouterRequest_t * message_router_request,
                                            CipMessageRouterResponse_t* message_router_response)
{
    return kCipGeneralStatusCodeSuccess;
}

template <class T>
CipStatus CIP_Object_template<T>::SetAttributeAll(CipMessageRouterRequest_t * message_router_request,
                                         CipMessageRouterResponse_t* message_router_response)
{
    return kCipGeneralStatusCodeSuccess;
}

template <class T>
void CIP_Object_template<T>::RegisterGenericClassAttributes()
{
    if (classAttrInfo.size() == 0)
    {
        //Register class attributes
        classAttrInfo.emplace(1, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "Revision"});
        classAttrInfo.emplace(2, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "Max Instance"});
        classAttrInfo.emplace(3, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "Number of Instances"});
        //classAttrInfo.emplace(4 , CipAttrInfo_t{kCipUsintUsint, SZ(identityRevision_t), kAttrFlagGetableSingleAndAll, "Optional attribute list"});
        //classAttrInfo.emplace(5 , CipAttrInfo_t{kCipWord  , SZ(CipWord), kAttrFlagGetableSingleAndAll, "Optional service list"});
        classAttrInfo.emplace(6, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll,
                                               "Maximum ID Number Class Attributes"});
        classAttrInfo.emplace(7, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll,
                                               "Maximum ID Number Instance Attributes"});

    }
}


#endif