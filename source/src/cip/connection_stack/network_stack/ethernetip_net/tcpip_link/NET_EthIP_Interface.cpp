/******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 *****************************************************************************/
#include <cstring>
#include "../../../ciptypes.hpp"
#include "NET_EthIP_Interface.hpp"
#include "../../../CIP_Common.hpp"
#include "../NET_Connection.hpp"

#define CIP_ETHERNETLINK_CLASS_CODE 0xF6

CipStatus NET_EthIP_Interface::ConfigureNetworkInterface(const char* ip_address,   const char* subnet_mask, const char* gateway)
{

    interface_configuration_.ip_address = inet_addr(ip_address);
    interface_configuration_.network_mask = inet_addr(subnet_mask);
    interface_configuration_.gateway = inet_addr(gateway);

    // calculate the CIP multicast address. The multicast address is calculated, not input
    CipUdint host_id = NET_Connection::endian_ntohl(interface_configuration_.ip_address) & ~NET_Connection::endian_ntohl(interface_configuration_.network_mask); // see CIP spec 3-5.3 for multicast address algorithm
    host_id -= 1;
    host_id &= 0x3ff;

    g_multicast_configuration.starting_multicast_address = htonl(ntohl(inet_addr("239.192.1.0")) + (host_id << 5));

    return kCipStatusOk;
}

void NET_EthIP_Interface::ConfigureDomainName(const char* domain_name)
{
    if (NULL != interface_configuration_.domain_name.string)
    {
        // if the string is already set to a value we have to free the resources
        // before we can set the new value in order to avoid memory leaks.

        //CipFree(interface_configuration_.domain_name.string);
        delete[] interface_configuration_.domain_name.string;
    }
    interface_configuration_.domain_name.length = strlen(domain_name);
    if (interface_configuration_.domain_name.length)
    {
        interface_configuration_.domain_name.string = new CipByte[interface_configuration_.domain_name.length+1]();//(CipByte*)CipCalloc(interface_configuration_.domain_name.length + 1, sizeof(EipInt8));
        strcpy((char*)interface_configuration_.domain_name.string, domain_name);
    }
    else
    {
        interface_configuration_.domain_name.string = NULL;
    }
}

void NET_EthIP_Interface::ConfigureHostName(const char* hostname)
{
    if (NULL != hostname_.string)
    {
        // if the string is already set to a value we have to free the resources
        // before we can set the new value in order to avoid memory leaks.

        //CipFree(hostname_.string);
    }
    hostname_.length = strlen(hostname);
    if (hostname_.length)
    {
        hostname_.string = new CipByte[hostname_.length+1]();//(CipByte*)CipCalloc(hostname_.length + 1, sizeof(CipByte));
        strcpy((char*)hostname_.string, hostname);
    }
    else
    {
        hostname_.string = NULL;
    }
}

CipStatus NET_EthIP_Interface::SetAttributeSingleTcp(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{
    CIP_Attribute* attribute = this->GetCipAttribute(message_router_request->request_path.attribute_number);

    if (0 != attribute)
    {
        // it is an attribute we currently support, however no attribute is setable
        // TODO: if you like to have a device that can be configured via this CIP object add your code here
        // TODO: check for flags associated with attributes
        message_router_response->general_status = kCipErrorAttributeNotSetable;
    }
    else
    {
        // we don't have this attribute
        message_router_response->general_status = kCipErrorAttributeNotSupported;
    }

    message_router_response->size_of_additional_status = 0;
    message_router_response->data_length = 0;
    message_router_response->reply_service = (0x80 | message_router_request->service);
    return kCipStatusOkSend;
}

CipStatus NET_EthIP_Interface::CipTcpIpInterfaceInit()
{
    NET_EthIP_Interface* instance;

    if (instancesNum == 0)
    {
        class_name = "TCP/IP interface";
        class_id = kCipTcpIpInterfaceClassCode;
        get_all_class_attributes_mask = 0xffffffff;
        get_all_instance_attributes_mask = 0xffffffff;

        tcp_status_ = 0x1;
        configuration_capability_ = 0x04 | 0x20;
        configuration_control_ = 0;
        physical_link_object_ = {2, CIP_ETHERNETLINK_CLASS_CODE, 1, 0};
        interface_configuration_ = { 0, 0, 0, 0, 0, {0, NULL,} };
        hostname_= { 0, NULL };
        g_multicast_configuration = {
                0, // us the default allocation algorithm
                0, // reserved
                1, // we currently use only one multicast address
                0 // the multicast address will be allocated on ip address configuration
        };
        g_time_to_live_value = 1;

        instance = new NET_EthIP_Interface();

        instance->InsertAttribute(1, kCipDword, (void*)&tcp_status_, kGetableSingleAndAll);
        instance->InsertAttribute(2, kCipDword, (void*)&configuration_capability_, kGetableSingleAndAll);
        instance->InsertAttribute(3, kCipDword, (void*)&configuration_control_, kGetableSingleAndAll);
        instance->InsertAttribute(4, kCipEpath, &physical_link_object_, kGetableSingleAndAll);
        instance->InsertAttribute(5, kCipUdintUdintUdintUdintUdintString, &interface_configuration_, kGetableSingleAndAll);
        instance->InsertAttribute(6, kCipString, (void*)&hostname_, kGetableSingleAndAll);
        instance->InsertAttribute(8, kCipUsint, (void*)&g_time_to_live_value, kGetableSingleAndAll);
        instance->InsertAttribute(9, kCipAny, (void*)&g_multicast_configuration, kGetableSingleAndAll);
    }
    else
    {
        return kCipStatusError;
    }


    return kCipStatusOk;
}

void NET_EthIP_Interface::ShutdownTcpIpInterface(void)
{
    //Only free the resources if they are initialized
    if (NULL != hostname_.string)
    {
        //CipFree(hostname_.string);
        delete[] hostname_.string;
        hostname_.string = NULL;
    }

    if (NULL != interface_configuration_.domain_name.string)
    {
        //CipFree(interface_configuration_.domain_name.string);
        interface_configuration_.domain_name.string = NULL;
    }
}

CipStatus NET_EthIP_Interface::GetAttributeSingleTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{

    CipStatus status = kCipStatusOkSend;
    CipByte* message = message_router_response->data;

    if (9 == message_router_request->request_path.attribute_number) { // attribute 9 can not be easily handled with the default mechanism therefore we will do it by hand
        message_router_response->data_length = 0;
        message_router_response->reply_service = (0x80 | message_router_request->service);
        message_router_response->general_status = kCipErrorSuccess;
        message_router_response->size_of_additional_status = 0;

        message_router_response->data_length += CIP_Common::EncodeData(kCipUsint, &(g_multicast_configuration.alloc_control), &message);
        message_router_response->data_length += CIP_Common::EncodeData(kCipUsint, &(g_multicast_configuration.reserved_shall_be_zero), &message);
        message_router_response->data_length += CIP_Common::EncodeData(kCipUint, &(g_multicast_configuration.number_of_allocated_multicast_addresses), &message);

        CipUdint multicast_address = ntohl(g_multicast_configuration.starting_multicast_address);

        message_router_response->data_length += CIP_Common::EncodeData(kCipUdint, &multicast_address, &message);
    }
    else
    {
        status = this->GetAttributeSingle(message_router_request, message_router_response);
    }
    return status;
}

CipStatus NET_EthIP_Interface::GetAttributeAllTcpIpInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{

    CipUsint* response = message_router_response->data; // pointer into the reply
    CIP_Attribute* attribute;

    for (int j = 0; j < this->attributes.size(); j++) // for each instance attribute of this class
    {
        int attribute_number = attribute[j].getNumber ();

        // only return attributes that are flagged as being part of GetAttributeALl
        if (attribute_number < 32 && (this->get_all_instance_attributes_mask & 1 << attribute_number))
        {
            message_router_request->request_path.attribute_number = attribute_number;

            if (8 == attribute_number)
            {
                // insert 6 zeros for the required empty safety network number according to Table 5-3.10
                memset(message_router_response->data, 0, 6);
                message_router_response->data += 6;
            }

            if (kCipStatusOkSend != this->GetAttributeSingleTcpIpInterface(message_router_request, message_router_response))
            {
                message_router_response->data = response;
                return kCipStatusError;
            }
            message_router_response->data += message_router_response->data_length;
        }
        attribute++;
    }
    message_router_response->data_length = message_router_response->data - response;
    message_router_response->data = response;

    return kCipStatusOkSend;
}

CipStatus NET_EthIP_Interface::InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response)
{
    CipStatus returnValue;
    //Class services
    if (this->id == 0)
    {
        switch(service)
        {
            case kGetAttributeSingle:
                returnValue = this->GetAttributeSingleTcpIpInterface (msg_router_request, msg_router_response);
                break;
            case kGetAttributeAll:
                returnValue = this->GetAttributeAllTcpIpInterface (msg_router_request, msg_router_response);
                break;
            case kSetAttributeSingle:
                returnValue = this->SetAttributeSingleTcp (msg_router_request, msg_router_response);
                break;
            default:
                returnValue = kCipStatusError;
        }

    }
    //Instance services
    else
    {
        returnValue = kCipStatusError;
    }
    return returnValue;
}
