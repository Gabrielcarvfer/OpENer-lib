/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <cstring> /*needed for memcpy */
#include <cstdlib>
#include "CIP_Assembly.hpp"
#include "connection/CIP_Connection.hpp"

CIP_Assembly::CIP_Assembly()
{
    id = GetNumberOfInstances();
    //TODO:Fix to emplace in the first empty slot instead of appending
    AddClassInstance(this, (CipUdint) id);
}

CIP_Assembly::~CIP_Assembly()
{
    RemoveClassInstance(this);
}

// create the CIP Assembly object with zero instances
CipStatus CIP_Assembly::CipAssemblyInitialize(void)
{
    class_id = kCipAssemblyClassCode;
    class_name = "Assembly";
    get_all_class_attributes_mask = 0;
    get_all_instance_attributes_mask = 0;
    revision = 0;

    CIP_Assembly *instance = new CIP_Assembly();

    object_Set.emplace(object_Set.size(), instance);

    return kCipStatusOk;
}

void CIP_Assembly::ShutdownAssemblies(void)
{
    if (CIP_Assembly::GetClass() != NULL)
    {
        CIP_Attribute* attribute;
        const CIP_Object* instance;

        for(unsigned int i = 1; i <= GetNumberOfInstances(); i++)
        {
            instance = GetInstance(i);

            attribute = ((CIP_Object*)instance)->GetCipAttribute(3);
            if (NULL != attribute)
            {
                //delete[] (attribute->getData());
                free(attribute->getData ());
            }
        }
    }
}

CIP_Assembly::CIP_Assembly(CipByte* data, CipUint data_length)
{
    CIP_Assembly* instance;
    CipByteArray* assembly_byte_array;

    /* add instances (always succeeds (or asserts))*/
    instance = new CIP_Assembly();

    if ((assembly_byte_array = new CipByteArray()) == NULL)
    {
        exit(-1);//return NULL; /*TODO remove assembly instance in case of error*/
    }

    assembly_byte_array->length = data_length;
    assembly_byte_array->data = data;
    instance->InsertAttribute(3, kCipByteArray, assembly_byte_array, kSetAndGetAble);

    /* Attribute 4 Number of bytes in Attribute 3 */
    instance->InsertAttribute(4, kCipUint, &(assembly_byte_array->length), kGetableSingle);
}

CipStatus CIP_Assembly::NotifyAssemblyConnectedDataReceived(CipUsint* data, CipUint data_length)
{
    CipByteArray* assembly_byte_array;

    /* empty path (path size = 0) need to be checked and taken care of in future */
    /* copy received data to Attribute 3 */
    assembly_byte_array = (decltype (assembly_byte_array))(this->attributes[2]->getData());
    if (assembly_byte_array->length != data_length)
    {
        OPENER_TRACE_ERR("wrong amount of data arrived for assembly object\n");
        return kCipStatusError; /*TODO question should we notify the application that wrong data has been recieved???*/
    }
    else
    {
        memcpy(assembly_byte_array->data, data, data_length);
        /* call the application that new data arrived */
    }

    return kCipStatusOk;//TODO:OpENer_Interface::AfterAssemblyDataReceived(this);
}

CipStatus CIP_Assembly::SetAssemblyAttributeSingle(CipMessageRouterRequest* message_router_request,
                                                  CipMessageRouterResponse* message_router_response)
{
    CipUsint* router_request_data;
    CIP_Attribute* attribute;
    OPENER_TRACE_INFO(" setAttribute %d\n", message_router_request->request_path.attribute_number);

    router_request_data = message_router_request->data;

    message_router_response->data_length = 0;
    message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
    message_router_response->general_status = kCipErrorAttributeNotSupported;
    message_router_response->size_of_additional_status = 0;

    attribute = this->GetCipAttribute(message_router_request->request_path.attribute_number);

    if ((attribute != NULL) && (3 == message_router_request->request_path.attribute_number))
    {
        if (attribute->getData() != NULL)
        {
            CipByteArray* data = (CipByteArray*)attribute->getData ();

            /* TODO: check for ATTRIBUTE_SET/GETABLE MASK */
            if (true == CIP_Connection::IsConnectedOutputAssembly((CipUdint) GetInstanceNumber(this)))
            {
                OPENER_TRACE_WARN("Assembly AssemblyAttributeSingle: received data for connected output assembly\n\r");
                message_router_response->general_status = kCipErrorAttributeNotSetable;
            }
            else
            {
                if (message_router_request->data_length < data->length)
                {
                    OPENER_TRACE_INFO("Assembly setAssemblyAttributeSingle: not enough data received.\r\n");
                    message_router_response->general_status = kCipErrorNotEnoughData;
                }
                else
                {
                    if (message_router_request->data_length > data->length)
                    {
                        OPENER_TRACE_INFO("Assembly setAssemblyAttributeSingle: too much data received.\r\n");
                        message_router_response->general_status = kCipErrorTooMuchData;
                    }
                    else
                    {
                        memcpy(data->data, router_request_data, data->length);

                        if (kCipStatusError)//OpENer_Interface::AfterAssemblyDataReceived(instance) != kCipStatusOk)
                        {
                            /* punt early without updating the status... though I don't know
               * how much this helps us here, as the attribute's data has already
               * been overwritten.
               *
               * however this is the task of the application side which will
               * take the data. In addition we have to inform the sender that the
               * data was not ok.
               */
                            message_router_response->general_status = kCipErrorInvalidAttributeValue;
                        }
                        else
                        {
                            message_router_response->general_status = kCipErrorSuccess;
                        }
                    }
                }
            }
        }
        else
        {
            /* the attribute was zero we are a heartbeat assembly */
            message_router_response->general_status = kCipErrorTooMuchData;
        }
    }

    if ((attribute != NULL) && (4 == message_router_request->request_path.attribute_number))
    {
        message_router_response->general_status = kCipErrorAttributeNotSetable;
    }

    return kCipStatusOkSend;
}

CipStatus CIP_Assembly::InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response)
{
    //Class services
    if (this->id == 0)
    {
        return this->SetAssemblyAttributeSingle(msg_router_request, msg_router_response);
    }
    //Instance services
    else
    {
        return kCipStatusError;
    }
}

