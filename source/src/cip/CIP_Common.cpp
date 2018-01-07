/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
//Includes

#include <CIP_Common.hpp>
#include <cstring>
#include <iostream>
#include <cip/CIP_Objects/CIP_Object.hpp>
#include <connection/CIP_CommonPacket.hpp>
#include <CIP_AppConnType.hpp>
#include <connection/network/NET_Endianconv.hpp>
#include <CIP_Objects/CIP_ClassStack.hpp>
//#include <CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp>
//#include <CIP_Objects/CIP_0001_Identity/CIP_Identity.hpp>
#include <CIP_Objects/CIP_0002_MessageRouter/CIP_MessageRouter.hpp>
//#include <CIP_Objects/CIP_0004_Assembly/CIP_Assembly.hpp>
#include <connection/network/NET_Encapsulation.hpp>

//Static variables

std::map <CipUsint,CipByteArray*> CIP_Common::message_data_reply_buffer;

//Methods
void CIP_Common::CipStackInit (CipUint unique_connection_id)
{
    CipStatus eip_status;
    NET_Encapsulation::Initialize ();

    int pointOfFail = 0; // OK

    pointOfFail = CIP_ClassStack::InitObjects();

    switch(pointOfFail)
    {
        case (0):
            break;
        default:
            std::cout << "Failed to initialize CIP Object with code " << pointOfFail << std::endl;
            exit(-1);

    }
}

void CIP_Common::ShutdownCipStack (void)
{
    /* First close all connections */
    CIP_AppConnType::CloseAllConnections ();

    int pointOfFail = CIP_ClassStack::ShutObjects();

    switch(pointOfFail)
    {
        case (0):
            break;
        default:
            std::cout << "Failed to shutdown CIP Object with code " << pointOfFail << std::endl;
            exit(-1);

    }

    NET_Encapsulation::Shutdown ();

    /*no clear all the instances and classes */
    //CIP_MessageRouter::DeleteAllClasses ();
}

//Just forwards getClass and getInstance to CIP_MessageRouter
CIP_Object_generic * CIP_Common::getClass(CipUint class_id)
{
    return CIP_MessageRouter::GetRegisteredObject(class_id);
}

//Just forwards getClass and getInstance to CIP_MessageRouter
CIP_Object_generic * CIP_Common::getClassInstance(CipUint class_id, CipUint instance_id)
{
    return (CIP_Object_generic*)CIP_MessageRouter::GetRegisteredObject(class_id)->glue.GetInstance(instance_id);
}

/*
CipStatus CIP_Common::NotifyClass (CipMessageRouterRequest_t *message_router_request, CipMessageRouterResponse_t *message_router_response)
{
    int i;
    CIP_ClassInstance *instance;
    std::map<CipUdint, CIP_Service *>service_set;
    unsigned instance_number; // my instance number

    // find the instance: if instNr==0, the class is addressed, else find the instance

    // get the instance number
    instance_number = message_router_request->request_path.instance_number;

    // look up the instance (note that if inst==0 this will be the class itself)
    instance = CIP_ClassInstance::GetCipClassInstance (CIP_ClassInstance->class_id, instance_number);

    if (instance) // if instance is found
    {
        OPENER_TRACE_INFO("notify: found instance %d%s\n", instance_number,
                          instance_number == 0 ? " (class object)" : "");

        service_set = CIP_ClassInstance::GetCipClass (instance->class_id)->services; // get pointer to array of services
        if (service_set.size()>0) // if services are defined
        {
            for (i = 0; i < service_set.size(); i++) // seach the services list
            {
                if (service_set.find(message_router_request->service) != service_set.end()) // if match is found
                {
                    // call the service, and return what it returns
                    OPENER_TRACE_INFO("notify: calling %s service\n", service->name);
                    //OPENER_ASSERT(nullptr != service_set[i]->service_function);
                    return service_set[i]->getService (instance, message_router_request, message_router_response);
                }
            }
        }
        OPENER_TRACE_WARN("notify: service 0x%x not supported\n", message_router_request->service);
        message_router_response->general_status = kCipGeneralStatusCodeServiceNotSupported; // if no services or service not found, return an error reply
    }
    else
    {
        OPENER_TRACE_WARN("notify: instance number %d unknown\n", instance_number);
        // if instance not found, return an error reply
        //according to the test tool this should be the correct error flag instead of CIP_ERROR_OBJECT_DOES_NOT_EXIST;
        message_router_response->general_status = kCipGeneralStatusCodePathDestinationUnknown;
    }

    // handle error replies

    // fill in the rest of the reply with not much of anything
    message_router_response->size_additional_status = 0;
    message_router_response->data_length = 0;

    // except the reply code is an echo of the command + the reply flag
    message_router_response->reply_service = (CipUsint)(0x80 | message_router_request->service);
    return kCipGeneralStatusCodeSuccess;
}
*/



int CIP_Common::EncodeEPath (CipEpath *epath, CipUsint *message)
{
    unsigned int length = epath->path_size;
    NET_Endianconv::AddIntToMessage (epath->path_size, message);

    if (epath->class_id < 256)
    {
        *message = 0x20; /*8Bit Class Id */
        ++(message);
        *message = (CipUsint) epath->class_id;
        ++(message);
        length -= 1;
    }
    else
    {
        *message = 0x21; /*16Bit Class Id */
        ++(message);
        *message = 0; /*pad byte */
        ++(message);
        NET_Endianconv::AddIntToMessage (epath->class_id, message);
        length -= 2;
    }

    if (0 < length)
    {
        if (epath->instance_number < 256)
        {
            *message = 0x24; /*8Bit Instance Id */
            ++(message);
            *message = (CipUsint) epath->instance_number;
            ++(message);
            length -= 1;
        }
        else
        {
            *message = 0x25; /*16Bit Instance Id */
            ++(message);
            *message = 0; /*padd byte */
            ++(message);
            NET_Endianconv::AddIntToMessage (epath->instance_number, message);
            length -= 2;
        }

        if (0 < length)
        {
            if (epath->attribute_number < 256)
            {
                *message = 0x30; /*8Bit Attribute Id */
                ++(message);
                *message = (CipUsint) epath->attribute_number;
                ++(message);
                length -= 1;
            }
            else
            {
                *message = 0x31; /*16Bit Attribute Id */
                ++(message);
                *message = 0; /*pad byte */
                ++(message);
                NET_Endianconv::AddIntToMessage (epath->attribute_number, message);
                length -= 2;
            }
        }
    }

    return 2 + epath->path_size * 2; /* path size is in 16 bit chunks according to the specification */
}

int CIP_Common::DecodePaddedEPath (CipEpath *epath, CipUsint *message)
{
    unsigned int number_of_decoded_elements;
    CipUsint *message_runner = message;

    epath->path_size = *message_runner;
    message_runner++;
    /* copy path to structure, in version 0.1 only 8 bit for Class,Instance and Attribute, need to be replaced with function */
    epath->class_id = 0;
    epath->instance_number = 0;
    epath->attribute_number = 0;

    for (number_of_decoded_elements = 0; number_of_decoded_elements < epath->path_size; number_of_decoded_elements++)
    {
        if (kSegmentTypeSegmentTypeReserved == ((*message_runner) & kSegmentTypeSegmentTypeReserved))
        {
            /* If invalid/reserved segment type, segment type greater than 0xE0 */
            return kCipStatusError;
        }

        switch (*message_runner)
        {
            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeClassId +
                 kLogicalSegmentLogicalFormatEightBitValue:
                epath->class_id = * (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeClassId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->class_id = NET_Endianconv::GetIntFromMessage (message_runner);
                number_of_decoded_elements++;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeInstanceId +
                 kLogicalSegmentLogicalFormatEightBitValue:
                epath->instance_number = * (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeInstanceId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->instance_number = NET_Endianconv::GetIntFromMessage (message_runner);
                number_of_decoded_elements++;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeAttributeId +
                 kLogicalSegmentLogicalFormatEightBitValue:
                epath->attribute_number = * (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeAttributeId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->attribute_number = NET_Endianconv::GetIntFromMessage (message_runner);
                number_of_decoded_elements++;
                break;

            default:
                OPENER_TRACE_ERR("wrong path requested\n");
                return kCipStatusError;
                break;
        }
    }

    message = message_runner;
    return number_of_decoded_elements * 2 + 1; /* i times 2 as every encoding uses 2 bytes */
}
