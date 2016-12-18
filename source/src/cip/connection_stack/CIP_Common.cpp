/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include <string.h>

#include <CIP_Common.h>

#include <CIP_CommonPacket.h>
#include <appcontype.h>
#include <NET_EthIP_Link.h>
#include <CIP_Identity.h>
#include <CIP_MessageRouter.h>
#include <NET_EthIP_Interface.h>
#include <UTIL_Endianconv.h>
#include <trace.h>
#include <opener_user_conf.h>

void CIP_Common::CipStackInit (CipUint unique_connection_id)
{
    CipStatus eip_status;
    EncapsulationInit ();

    /* The message router is the first CIP object be initialized!!! */
    eip_status = CIP_MessageRouter::CipMessageRouterInit ();
    OPENER_ASSERT(kCipStatusOk == eip_status);

    eip_status = CIP_Identity::CipIdentityInit ();
    OPENER_ASSERT(kCipStatusOk == eip_status);

    eip_status = NET_EthIP_Interface::CipTcpIpInterfaceInit ();
    OPENER_ASSERT(kCipStatusOk == eip_status);

    eip_status = NET_EthIP_Link::CipEthernetLinkInit ();
    OPENER_ASSERT(kCipStatusOk == eip_status);

    eip_status = CIP_Connection::ConnectionManagerInit (unique_connection_id);
    OPENER_ASSERT(kCipStatusOk == eip_status);

    eip_status = CIP_Assembly::CipAssemblyInitialize ();
    OPENER_ASSERT(kCipStatusOk == eip_status);

    /* the application has to be initialized at last */
    //eip_status = ApplicationInitialization ();
    //OPENER_ASSERT(kCipStatusOk == eip_status);
}

void CIP_Common::ShutdownCipStack (void)
{
    /* First close all connections */
    CloseAllConnections ();
    /* Than free the sockets of currently active encapsulation sessions */
    EncapsulationShutDown ();
    /*clean the data needed for the assembly object's attribute 3*/
    CIP_Assembly::ShutdownAssemblies ();

    NET_EthIP_Interface::ShutdownTcpIpInterface ();

    /*no clear all the instances and classes */
    CIP_MessageRouter::DeleteAllClasses ();
}
/*
CipStatus CIP_Common::NotifyClass (CipMessageRouterRequest *message_router_request, CipMessageRouterResponse *message_router_response)
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
                    //OPENER_ASSERT(NULL != service_set[i]->service_function);
                    return service_set[i]->getService (instance, message_router_request, message_router_response);
                }
            }
        }
        OPENER_TRACE_WARN("notify: service 0x%x not supported\n", message_router_request->service);
        message_router_response->general_status = kCipErrorServiceNotSupported; // if no services or service not found, return an error reply
    }
    else
    {
        OPENER_TRACE_WARN("notify: instance number %d unknown\n", instance_number);
        // if instance not found, return an error reply
        //according to the test tool this should be the correct error flag instead of CIP_ERROR_OBJECT_DOES_NOT_EXIST;
        message_router_response->general_status = kCipErrorPathDestinationUnknown;
    }

    // handle error replies

    // fill in the rest of the reply with not much of anything
    message_router_response->size_of_additional_status = 0;
    message_router_response->data_length = 0;

    // except the reply code is an echo of the command + the reply flag
    message_router_response->reply_service = (CipUsint)(0x80 | message_router_request->service);
    return kCipStatusOkSend;
}
*/
int CIP_Common::EncodeData (CipUsint cip_type, void *data, CipUsint **message)
{
    int counter = 0;

    switch (cip_type)
        /* check the data type of attribute */
    {
        case (kCipBool):
        case (kCipSint):
        case (kCipUsint):
        case (kCipByte):
            **message = *(CipUsint *) (data);
            ++(*message);
            counter = 1;
            break;

        case (kCipInt):
        case (kCipUint):
        case (kCipWord):
            UTIL_Endianconv::AddIntToMessage (*(CipUint *) (data), message);
            counter = 2;
            break;

        case (kCipDint):
        case (kCipUdint):
        case (kCipDword):
        case (kCipReal):
            UTIL_Endianconv::AddDintToMessage (*(CipUdint *) (data), message);
            counter = 4;
            break;

#ifdef OPENER_SUPPORT_64BIT_DATATYPES
        case (kCipLint):
        case (kCipUlint):
        case (kCipLword):
        case (kCipLreal):
            AddLintToMessage(*(CipUlint*)(data), message);
            counter = 8;
            break;
#endif

        case (kCipStime):
        case (kCipDate):
        case (kCipTimeOfDay):
        case (kCipDateAndTime):
            break;
        case (kCipString):
        {
            CipString *string = (CipString *) data;

            UTIL_Endianconv::AddIntToMessage (* &(string->length), message);
            memcpy (*message, string->string, string->length);
            *message += string->length;

            counter = string->length + 2; /* we have a two byte length field */
            if (counter & 0x01)
            {
                /* we have an odd byte count */
                **message = 0;
                ++(*message);
                counter++;
            }
            break;
        }
        case (kCipString2):
        case (kCipFtime):
        case (kCipLtime):
        case (kCipItime):
        case (kCipStringN):
            break;

        case (kCipShortString):
        {
            CipShortString *short_string = (CipShortString *) data;

            **message = short_string->length;
            ++(*message);

            memcpy (*message, short_string->string, short_string->length);
            *message += short_string->length;

            counter = short_string->length + 1;
            break;
        }

        case (kCipTime):
            break;

        case (kCipEpath):
            counter = EncodeEPath ((CipEpath *) data, message);
            break;

        case (kCipEngUnit):
            break;

        case (kCipUsintUsint):
        {
            CipRevision *revision = (CipRevision *) data;

            **message = revision->major_revision;
            ++(*message);
            **message = revision->minor_revision;
            ++(*message);
            counter = 2;
            break;
        }

        case (kCipUdintUdintUdintUdintUdintString):
        {
            /* TCP/IP attribute 5 */
            CipTcpIpNetworkInterfaceConfiguration *tcp_ip_network_interface_configuration = (CipTcpIpNetworkInterfaceConfiguration *) data;
            UTIL_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->ip_address), message);
            UTIL_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->network_mask), message);
            UTIL_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->gateway), message);
            UTIL_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->name_server), message);
            UTIL_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->name_server_2), message);
            counter = 20;
            counter += EncodeData (kCipString, &(tcp_ip_network_interface_configuration->domain_name), message);
            break;
        }

        case (kCip6Usint):
        {
            CipUsint *p = (CipUsint *) data;
            memcpy (*message, p, 6);
            counter = 6;
            break;
        }

        case (kCipMemberList):
            break;

        case (kCipByteArray):
        {
            CipByteArray *cip_byte_array;
            OPENER_TRACE_INFO(" -> get attribute byte array\r\n");
            cip_byte_array = (CipByteArray *) data;
            memcpy (*message, cip_byte_array->data, cip_byte_array->length);
            *message += cip_byte_array->length;
            counter = cip_byte_array->length;
        }
            break;

        case (kInternalUint6): /* TODO for port class attribute 9, hopefully we can find a better way to do this*/
        {
            CipUint *internal_unit16_6 = (CipUint *) data;

            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[0], message);
            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[1], message);
            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[2], message);
            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[3], message);
            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[4], message);
            UTIL_Endianconv::AddIntToMessage (internal_unit16_6[5], message);
            counter = 12;
            break;
        }
        default:
            break;
    }

    return counter;
}

int CIP_Common::DecodeData (CipUsint cip_type, void *data, CipUsint **message)
{
    int number_of_decoded_bytes = -1;

    switch (cip_type)
        /* check the data type of attribute */
    {
        case (kCipBool):
        case (kCipSint):
        case (kCipUsint):
        case (kCipByte):
            *(CipUsint *) (data) = **message;
            ++(*message);
            number_of_decoded_bytes = 1;
            break;

        case (kCipInt):
        case (kCipUint):
        case (kCipWord):
            (*(CipUint *) (data)) = UTIL_Endianconv::GetIntFromMessage (message);
            number_of_decoded_bytes = 2;
            break;

        case (kCipDint):
        case (kCipUdint):
        case (kCipDword):
            (*(CipUdint *) (data)) = UTIL_Endianconv::GetDintFromMessage (message);
            number_of_decoded_bytes = 4;
            break;

#ifdef OPENER_SUPPORT_64BIT_DATATYPES
        case (kCipLint):
        case (kCipUlint):
        case (kCipLword): {
            (*(CipUlint*)(data)) = GetLintFromMessage(message);
            number_of_decoded_bytes = 8;
        } break;
#endif

        case (kCipString):
        {
            CipString *string = (CipString *) data;
            string->length = UTIL_Endianconv::GetIntFromMessage (message);
            memcpy (string->string, *message, string->length);
            *message += string->length;

            number_of_decoded_bytes = string->length + 2; /* we have a two byte length field */
            if (number_of_decoded_bytes & 0x01)
            {
                /* we have an odd byte count */
                ++(*message);
                number_of_decoded_bytes++;
            }
        }
            break;
        case (kCipShortString):
        {
            CipShortString *short_string = (CipShortString *) data;

            short_string->length = **message;
            ++(*message);

            memcpy (short_string->string, *message, short_string->length);
            *message += short_string->length;

            number_of_decoded_bytes = short_string->length + 1;
            break;
        }

        default:
            break;
    }

    return number_of_decoded_bytes;
}


int CIP_Common::EncodeEPath (CipEpath *epath, CipUsint **message)
{
    unsigned int length = epath->path_size;
    UTIL_Endianconv::AddIntToMessage (epath->path_size, message);

    if (epath->class_id < 256)
    {
        **message = 0x20; /*8Bit Class Id */
        ++(*message);
        **message = (CipUsint) epath->class_id;
        ++(*message);
        length -= 1;
    }
    else
    {
        **message = 0x21; /*16Bit Class Id */
        ++(*message);
        **message = 0; /*pad byte */
        ++(*message);
        UTIL_Endianconv::AddIntToMessage (epath->class_id, message);
        length -= 2;
    }

    if (0 < length)
    {
        if (epath->instance_number < 256)
        {
            **message = 0x24; /*8Bit Instance Id */
            ++(*message);
            **message = (CipUsint) epath->instance_number;
            ++(*message);
            length -= 1;
        }
        else
        {
            **message = 0x25; /*16Bit Instance Id */
            ++(*message);
            **message = 0; /*padd byte */
            ++(*message);
            UTIL_Endianconv::AddIntToMessage (epath->instance_number, message);
            length -= 2;
        }

        if (0 < length)
        {
            if (epath->attribute_number < 256)
            {
                **message = 0x30; /*8Bit Attribute Id */
                ++(*message);
                **message = (CipUsint) epath->attribute_number;
                ++(*message);
                length -= 1;
            }
            else
            {
                **message = 0x31; /*16Bit Attribute Id */
                ++(*message);
                **message = 0; /*pad byte */
                ++(*message);
                UTIL_Endianconv::AddIntToMessage (epath->attribute_number, message);
                length -= 2;
            }
        }
    }

    return 2 + epath->path_size * 2; /* path size is in 16 bit chunks according to the specification */
}

int CIP_Common::DecodePaddedEPath (CipEpath *epath, CipUsint **message)
{
    unsigned int number_of_decoded_elements;
    CipUsint *message_runner = *message;

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
                epath->class_id = *(CipUsint *) (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeClassId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->class_id = UTIL_Endianconv::GetIntFromMessage (&(message_runner));
                number_of_decoded_elements++;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeInstanceId +
                 kLogicalSegmentLogicalFormatEightBitValue:
                epath->instance_number = *(CipUsint *) (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeInstanceId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->instance_number = UTIL_Endianconv::GetIntFromMessage (&(message_runner));
                number_of_decoded_elements++;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeAttributeId +
                 kLogicalSegmentLogicalFormatEightBitValue:
                epath->attribute_number = *(CipUsint *) (message_runner + 1);
                message_runner += 2;
                break;

            case kSegmentTypeLogicalSegment + kLogicalSegmentLogicalTypeAttributeId +
                 kLogicalSegmentLogicalFormatSixteenBitValue:
                message_runner += 2;
                epath->attribute_number = UTIL_Endianconv::GetIntFromMessage (&(message_runner));
                number_of_decoded_elements++;
                break;

            default:
                OPENER_TRACE_ERR("wrong path requested\n");
                return kCipStatusError;
                break;
        }
    }

    *message = message_runner;
    return number_of_decoded_elements * 2 + 1; /* i times 2 as every encoding uses 2 bytes */
}
