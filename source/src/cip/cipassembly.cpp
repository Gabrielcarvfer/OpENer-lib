/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include "cipassembly.h"
#include "cipcommon.h"
#include "cipconnectionmanager.h"
#include "opener_api.h"
#include "trace.h"
#include <string> /*needed for memcpy */


CIPClass* CIPAssembly::CreateAssemblyClass(void)
{
    CIPClass* assembly_class;
    /* create the CIP Assembly object with zero instances */
    assembly_class = CIPClass(kCipAssemblyClassCode, 
                                                  0, /* # class attributes*/
                                                  0, /* 0 as the assembly object should not have a get_attribute_all service*/
                                                  0, /* # class services*/
                                                  2, /* # instance attributes*/
                                                  0, /* 0 as the assembly object should not have a get_attribute_all service*/
                                                  1, /* # instance services*/
                                                  0, /* # instances*/
                                         "assembly", /* name */
                                                  2 /* Revision, according to the CIP spec currently this has to be 2 */
                              );

    if (NULL != assembly_class) 
        InsertService(assembly_class, kSetAttributeSingle, &SetAssemblyAttributeSingle, "SetAssemblyAttributeSingle");

    return assembly_class;
}

CipStatus CIPAssembly::CipAssemblyInitialize(void)
{ /* create the CIP Assembly object with zero instances */
    return (NULL != CreateAssemblyClass()) ? kCipStatusOk : kCipStatusError;
}

void CIPAssembly::ShutdownAssemblies(void)
{
    CIPClass* assembly_class = GetCIPClass(kCipAssemblyClassCode);
    CipAttributeStruct* attribute;
    CipInstance* instance;

    if (NULL != assembly_class) {
        instance = assembly_class->instances;
        while (NULL != instance) {
            attribute = GetCipAttribute(instance, 3);
            if (NULL != attribute) {
                CipFree(attribute->data);
            }
            instance = instance->next;
        }
    }
}

CipInstance* CIPAssembly::CreateAssemblyObject(CipUdint instance_id, EipByte* data,
    CipUint data_length)
{
    CIPClass* assembly_class;
    CipInstance* instance;
    CipByteArray* assembly_byte_array;

    if (NULL == (assembly_class = GetCIPClass(kCipAssemblyClassCode))) {
        if (NULL == (assembly_class = CreateAssemblyClass())) {
            return NULL;
        }
    }

    instance = new CIPClass(assembly_class, instance_id); /* add instances (always succeeds (or asserts))*/

    if ((assembly_byte_array = (CipByteArray*)CipCalloc(1, sizeof(CipByteArray)))
        == NULL) {
        return NULL; /*TODO remove assembly instance in case of error*/
    }

    assembly_byte_array->length = data_length;
    assembly_byte_array->data = data;
    InsertAttribute(instance, 3, kCipByteArray, assembly_byte_array,
        kSetAndGetAble);
    /* Attribute 4 Number of bytes in Attribute 3 */
    InsertAttribute(instance, 4, kCipUint, &(assembly_byte_array->length),
        kGetableSingle);

    return instance;
}

CipStatus CIPAssembly::NotifyAssemblyConnectedDataReceived(CipInstance* instance,
    CipUsint* data,
    CipUint data_length)
{
    CipByteArray* assembly_byte_array;

    /* empty path (path size = 0) need to be checked and taken care of in future */
    /* copy received data to Attribute 3 */
    assembly_byte_array = (CipByteArray*)instance->attributes->data;
    if (assembly_byte_array->length != data_length) {
        OPENER_TRACE_ERR("wrong amount of data arrived for assembly object\n");
        return kCipStatusError; /*TODO question should we notify the application that wrong data has been recieved???*/
    } else {
        memcpy(assembly_byte_array->data, data, data_length);
        /* call the application that new data arrived */
    }

    return AfterAssemblyDataReceived(instance);
}

CipStatus CIPAssembly::SetAssemblyAttributeSingle(CipInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{
    CipUsint* router_request_data;
    CipAttributeStruct* attribute;
    OPENER_TRACE_INFO(" setAttribute %d\n", message_router_request->request_path.attribute_number);

    router_request_data = message_router_request->data;

    message_router_response->data_length = 0;
    message_router_response->reply_service = (0x80 | message_router_request->service);
    message_router_response->general_status = kCipErrorAttributeNotSupported;
    message_router_response->size_of_additional_status = 0;

    attribute = GetCipAttribute(instance, message_router_request->request_path.attribute_number);

    if ((attribute != NULL) && (3 == message_router_request->request_path.attribute_number)) 
    {
        if (attribute->data != NULL) {
            CipByteArray* data = (CipByteArray*)attribute->data;

            /* TODO: check for ATTRIBUTE_SET/GETABLE MASK */
            if (true == IsConnectedOutputAssembly(instance->instance_number)) 
            {
                OPENER_TRACE_WARN(
                    "Assembly AssemblyAttributeSingle: received data for connected output assembly\n\r");
                message_router_response->general_status = kCipErrorAttributeNotSetable;
            } else 
            {
                if (message_router_request->data_length < data->length) 
                {
                    OPENER_TRACE_INFO(
                        "Assembly setAssemblyAttributeSingle: not enough data received.\r\n");
                    message_router_response->general_status = kCipErrorNotEnoughData;
                } else 
                {
                    if (message_router_request->data_length > data->length) 
                    {
                        OPENER_TRACE_INFO(
                            "Assembly setAssemblyAttributeSingle: too much data received.\r\n");
                        message_router_response->general_status = kCipErrorTooMuchData;
                    } else 
                    {
                        memcpy(data->data, router_request_data, data->length);

                        if (AfterAssemblyDataReceived(instance) != kCipStatusOk) 
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
                        } else 
                        {
                            message_router_response->general_status = kCipErrorSuccess;
                        }
                    }
                }
            }
        } else 
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
