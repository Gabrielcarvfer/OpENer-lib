/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <cstring> /*needed for memcpy */
#include <cstdlib>
#include "CIP_Assembly.hpp"
#include "cip/CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"

// create the CIP Assembly object with zero instances
CipStatus CIP_Assembly::Init(void)
{
    CipStatus stat;
    if (number_of_instances == 0)
    {
        class_id = kCipAssemblyClassCode;
        class_name = "Assembly";
        revision = 0;

        CIP_Assembly *instance = new CIP_Assembly();

        object_Set.emplace(object_Set.size(), instance);

        instance->classAttrInfo.emplace(3, CipAttrInfo_t{kCipByteArray, sizeof(CipByteArray), kAttrFlagGetableSingle, "AssemblyByteArray"});

        /* Attribute 4 Number of bytes in Attribute 3 */
        instance->classAttrInfo.emplace(4, CipAttrInfo_t{kCipUint, sizeof(CipUint), kAttrFlagGetableSingle, "AssemblyByteArrayLength"});

        stat.status = kCipStatusOk;
    }
    else
    {
        stat.status = kCipStatusError;
    }
    return stat;
}

CipStatus CIP_Assembly::Shut(void)
{

    CipStatus stat;
    stat.status = kCipGeneralStatusCodeSuccess;
    return stat;
}

CipStatus CIP_Assembly::Create(CipByte* data, CipUint data_length)
{
	CipStatus stat;
    CIP_Assembly* instance;

    /* add instances (always succeeds (or asserts))*/
    instance = new CIP_Assembly();

    instance->assemblyByteArray.length = data_length;
    instance->assemblyByteArray.data   = data;

	return stat;
}

CipStatus CIP_Assembly::NotifyAssemblyConnectedDataReceived(CipUsint* data, CipUint data_length)
{
	CipByteArray* assemblyByteArray = nullptr;;

    /* empty path (path size = 0) need to be checked and taken care of in future */
    /* copy received data to Attribute 3 */
    //assemblyByteArray = (decltype (assemblyByteArray))(this->attributes[2]->getData());
    if (assemblyByteArray->length != data_length)
    {
        OPENER_TRACE_ERR("wrong amount of data arrived for assembly object\n");
        return kCipStatusError; /*TODO question should we notify the application that wrong data has been recieved???*/
    }
    else
    {
        memcpy(assemblyByteArray->data, data, data_length);
        /* call the application that new data arrived */
    }

    return kCipGeneralStatusCodeSuccess;//TODO:OpENer_Interface::AfterAssemblyDataReceived(this);
}
/*
CipStatus CIP_Assembly::SetAssemblyAttributeSingle(CipMessageRouterRequest_t* message_router_request,
                                                  CipMessageRouterResponse_t* message_router_response)
{
    CipUsint* router_request_data;
    CIP_Attribute* attribute;
    OPENER_TRACE_INFO(" setAttribute %d\n", message_router_request->request_path.attribute_number);

    router_request_data = &message_router_request->request_data[0];
    message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
    message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSupported;
    message_router_response->size_additional_status = 0;

    attribute = this->GetCipAttribute(message_router_request->request_path.attribute_number);

    if ((attribute != nullptr) && (3 == message_router_request->request_path.attribute_number))
    {
        if (attribute->getData() != nullptr)
        {
            CipByteArray* data = (CipByteArray*)attribute->getData ();

            // TODO: check for ATTRIBUTE_SET/GETABLE MASK
            //if (CIP_ConnectionManager::IsConnectedOutputAssembly((CipUdint) GetInstanceNumber(this)) != 0)
            {
                OPENER_TRACE_WARN("Assembly AssemblyAttributeSingle: received data for connected output assembly\n\r");
                message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSetable;
            }
            //else
            {
                if (message_router_request->request_data.size() < data->length)
                {
                    OPENER_TRACE_INFO("Assembly setAssemblyAttributeSingle: not enough data received.\r\n");
                    message_router_response->general_status = kCipGeneralStatusCodeNotEnoughData;
                }
                else
                {
                    if (message_router_request->request_data.size() > data->length)
                    {
                        OPENER_TRACE_INFO("Assembly setAssemblyAttributeSingle: too much data received.\r\n");
                        message_router_response->general_status = kCipGeneralStatusCodeTooMuchData;
                    }
                    else
                    {
                        memcpy(data->data, router_request_data, data->length);

                        if (kCipStatusError)//OpENer_Interface::AfterAssemblyDataReceived(instance) != kCipGeneralStatusCodeSuccess)
                        {
                            // punt early without updating the status... though I don't know
               //how much this helps us here, as the attribute's data has already
               // been overwritten.
               //
               // however this is the task of the application side which will
               // take the data. In addition we have to inform the sender that the
               // data was not ok.
               //
                            message_router_response->general_status = kCipGeneralStatusCodeInvalidAttributeValue;
                        }
                        else
                        {
                            message_router_response->general_status = kCipGeneralStatusCodeSuccess;
                        }
                    }
                }
            }
        }
        else
        {
            // the attribute was zero we are a heartbeat assembly
            message_router_response->general_status = kCipGeneralStatusCodeTooMuchData;
        }
    }

    if ((attribute != nullptr) && (4 == message_router_request->request_path.attribute_number))
    {
        message_router_response->general_status = kCipGeneralStatusCodeAttributeNotSetable;
    }

    return kCipGeneralStatusCodeSuccess;
}

*/
void * CIP_Assembly::retrieveAttribute(CipUsint attributeNumber)
{
    if (this->id == 0)
    {
        switch(attributeNumber)
        {
			case 1:
            default:
                return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

CipStatus CIP_Assembly::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{
	CipStatus stat;
	return stat;
}