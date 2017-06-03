//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 18/11/2016.
// (code fom GetAttribute functions originally from OpENer)
//
#ifndef CIP_OBJECT_IMPL_H
#define CIP_OBJECT_IMPL_H

#include "../../../trace.hpp"
#include "../../CIP_Common.hpp"
#include "../../../opener_user_conf.hpp"
#include <utility>
#include "../../ciptypes.hpp"

//Static variables
template<class T> CipUdint    CIP_Object<T>::class_id;
template<class T> std::string CIP_Object<T>::class_name;
template<class T> CipUint     CIP_Object<T>::revision;
template<class T> CipUint     CIP_Object<T>::max_instances;
template<class T> CipUint     CIP_Object<T>::number_of_instances;
template<class T> CipUdint    CIP_Object<T>::optional_attribute_list;
template<class T> CipUdint    CIP_Object<T>::optional_service_list;
template<class T> CipUint     CIP_Object<T>::maximum_id_number_class_attributes;
template<class T> CipUint     CIP_Object<T>::maximum_id_number_instance_attributes;
template<class T> std::map<CipUdint, const T *> CIP_Object<T>::object_Set;

template<class T> std::map<CipUsint, CipAttrInfo_t> CIP_Object<T>::classAttrInfo;
template<class T> std::map<CipUsint, CipServiceProperties_t>   CIP_Object<T>::classServicesProperties;
template<class T> std::map<CipUsint, CipAttrInfo_t> CIP_Object<T>::instAttrInfo;
template<class T> std::map<CipUsint, CipServiceProperties_t>   CIP_Object<T>::instanceServicesProperties;

//Methods
template <class T>
CIP_Object<T>::CIP_Object()
{
  id = number_of_instances;
  number_of_instances++;
}

template <class T>
CIP_Object<T>::~CIP_Object()
{

}

template <class T>
const T * CIP_Object<T>::GetInstance(CipUdint instance_number)
{
    if (object_Set.size() >= instance_number)
        return object_Set[instance_number];
    else
        return nullptr;
}

template <class T>
const T  * CIP_Object<T>::GetClass()
{
    return GetInstance(0);
}

template <class T>
CipUdint CIP_Object<T>::GetNumberOfInstances()
{
    return (CipUdint)object_Set.size();
}

template <class T>
CipDint CIP_Object<T>::GetInstanceNumber(const T  * instance)
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
bool CIP_Object<T>::AddClassInstance(T  * instance, CipDint position)
{
    //If passed position is -1, then search first free position
    if (position == -1)
    {
        int i;
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
bool CIP_Object<T>::RemoveClassInstance(T  * instance)
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
bool CIP_Object<T>::RemoveClassInstance(CipUdint position)
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
void * CIP_Object<T>::GetCipAttribute(CipUsint attribute_number)
{
	bool isClass = this->id == 0;
	if (isClass)
	{
		if(classAttrInfo.find(attribute_number) != classAttrInfo.end())
			return this->retrieveAttribute(attribute_number);
	}
	else
	{
		if (instAttrInfo.find(attribute_number) != instAttrInfo.end())
			return this->retrieveAttribute(attribute_number);
	}

	OPENER_TRACE_WARN("attribute %d not defined\n", attribute_number);
	return nullptr;
}

template <class T>
CipStatus CIP_Object<T>::GetAttributeSingle(CipMessageRouterRequest_t* message_router_request,
                                            CipMessageRouterResponse_t* message_router_response)
{
    // Mask for filtering get-ability
    CipByte get_mask;

    CipAttrInfo_t* attribute = &instAttrInfo.at(message_router_request->request_path.attribute_number);
    CipByte* message = (CipByte*)&message_router_response->response_data[0];

    message_router_response->reply_service = (0x80 | message_router_request->service);
    message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSupported;
    message_router_response->size_additional_status = 0;

    // set filter according to service: get_attribute_all or get_attribute_single
    if (kGetAttributeAll == message_router_request->service)
    {
        get_mask = kGetableAll;
        message_router_response->general_status = kCipGeneralStatusCodeSuccess;
    }
    else
    {
        get_mask = kGetableSingle;
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
            //message_router_response->data_length = (CipInt)CIP_Common::EncodeData(attribute->getType(), attribute->getData(), &message);
            message_router_response->general_status = kCipGeneralStatusCodeSuccess;
        }
    }

    return CipStatus(kCipGeneralStatusCodeSuccess);
}

template <class T>
CipStatus CIP_Object<T>::GetAttributeAll(CipMessageRouterRequest_t* message_router_request,
                                         CipMessageRouterResponse_t* message_router_response)
{
    CipUsint* reply;

    // pointer into the reply
    reply = (CipUsint*) &message_router_response->response_data[0];

    if (this->id == 2)
    {
        OPENER_TRACE_INFO("GetAttributeAll: instance number 2\n");
    }

    CipServiceProperties_t * serviceProperties;
    CipAttrInfo_t * attributeProperties;

    auto it = instanceServicesProperties.find(kGetAttributeAll);

    if ( it != instanceServicesProperties.end())
    {
        serviceProperties = it->second;
        if (0 == this->attributes.size())
        {
            //there are no attributes to be sent back
            message_router_response->reply_service = (0x80 | message_router_request->service);
            message_router_response->general_status = kCipGeneralStatusCodeServiceNotSupported;
            message_router_response->size_additional_status = 0;
        }
        else
        {
            auto attrIt = instAttrInfo.begin ();
            for (; attrIt != instAttrInfo.end(); attrIt++) // for each instance attribute of this class
            {
                attributeProperties = attrIt->second;
                int attrNum = attrIt->first;

                // only return attributes that are flagged as being part of GetAttributeALl
                if (attrNum < 32 && (class_ptr->get_all_class_attributes_mask & 1 << attrNum))
                {
                    message_router_request->request_path.attribute_number = attrNum;
                    if (kCipGeneralStatusCodeSuccess != this->InstanceServices(kGetAttributeAll, message_router_request, message_router_response).status)
                    {
                        message_router_response->response_data->emplace (message_router_response->response_data->begin (), reply);

                        return CipStatus(kCipStatusError);
                    }
                    //message_router_response->data += message_router_response->data_length;
                }
            }
            //message_router_response->data_length = message_router_response->data - reply;
            message_router_response->response_data->emplace (message_router_response->response_data->begin (), reply);
        }
        return CipStatus(kCipGeneralStatusCodeSuccess);
    }
    return CipStatus(kCipGeneralStatusCodeSuccess); // Return kCipGeneralStatusCodeSuccess if cannot find GET_ATTRIBUTE_SINGLE service
}

template <class T>
CipStatus CIP_Object<T>::InstanceServices(int service, CipMessageRouterRequest_t* msg_router_request,
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
CipStatus CIP_Object<T>::SetAttributeSingle(CipMessageRouterRequest_t * message_router_request,
                                            CipMessageRouterResponse_t* message_router_response)
{
    return kCipGeneralStatusCodeSuccess;
}

template <class T>
CipStatus CIP_Object<T>::SetAttributeAll(CipMessageRouterRequest_t * message_router_request,
                                         CipMessageRouterResponse_t* message_router_response)
{
    return kCipGeneralStatusCodeSuccess;
}

template <class T>
void CIP_Object<T>::RegisterGenericClassAttributes()
{
    if (classAttrInfo.size() == 0)
    {
        //Register class attributes
        classAttrInfo.emplace(1, CipAttrInfo_t{kCipUint, SZ(CipUint), kGetableSingleAndAll, "Revision"});
        classAttrInfo.emplace(2, CipAttrInfo_t{kCipUint, SZ(CipUint), kGetableSingleAndAll, "Max Instance"});
        classAttrInfo.emplace(3, CipAttrInfo_t{kCipUint, SZ(CipUint), kGetableSingleAndAll, "Number of Instances"});
        //classAttrInfo.emplace(4 , CipAttrInfo_t{kCipUsintUsint, SZ(identityRevision_t), kGetableSingleAndAll, "Optional attribute list"});
        //classAttrInfo.emplace(5 , CipAttrInfo_t{kCipWord  , SZ(CipWord), kGetableSingleAndAll, "Optional service list"});
        classAttrInfo.emplace(6, CipAttrInfo_t{kCipUint, SZ(CipUint), kGetableSingleAndAll,
                                               "Maximum ID Number Class Attributes"});
        classAttrInfo.emplace(7, CipAttrInfo_t{kCipUint, SZ(CipUint), kGetableSingleAndAll,
                                               "Maximum ID Number Instance Attributes"});
    }
}


#endif