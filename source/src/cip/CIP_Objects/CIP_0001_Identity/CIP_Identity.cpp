/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

/**
 * @file cipidentity.c
 *
 * CIP Identity Object
 * ===================
 *
 * Implemented Attributes
 * ----------------------
 * - Attribute 1: VendorID
 * - Attribute 2: Device Type
 * - Attribute 3: Product Code
 * - Attribute 4: Revision
 * - Attribute 5: Status
 * - Attribute 6: Serial Number
 * - Attribute 7: Product Name
 *
 * Implemented Services
 * --------------------
 */

#include "CIP_Identity.hpp"
#include "../../../OpENer_Interface.hpp"

//Static variables
CipUint         CIP_Identity::vendor_id_;
CipUint         CIP_Identity::device_type_;
CipUint         CIP_Identity::product_code_;
CipRevision     CIP_Identity::revision_;
CipUint         CIP_Identity::status_;
CipUdint        CIP_Identity::serial_number_;
CipShortString  CIP_Identity::product_name_;

//Methods

/** Private functions, sets the devices serial number
 * @param serial_number The serial number of the device
 */
void CIP_Identity::SetDeviceSerialNumber(CipUdint serial_number)
{
    serial_number_ = serial_number;
}

/** Private functions, sets the devices status
 * @param status The serial number of the deivce
 */
void CIP_Identity::SetDeviceStatus(CipUint status)
{
    status_ = status;
}

/** Reset service
 *
 * @param instance
 * @param message_router_request
 * @param message_router_response
 * @returns Currently always kEipOkSend is returned
 */
 //                           pointer to message router request         -      pointer to message router response
CipStatus CIP_Identity::Reset( CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)
{
    CipStatus eip_status;

    eip_status = kCipGeneralStatusCodeSuccess;

    message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
    message_router_response->size_additional_status = 0;
    message_router_response->general_status = kCipGeneralStatusCodeSuccess;

    if (message_router_request->request_data.size() == 1)
    {
        switch (message_router_request->request_data[0])
        {
        case 0: /* Reset type 0 -> emulate device reset / Power cycle */
            if (kCipStatusError == OpENer_Interface::ResetDevice().status)
            {
                message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
            }
            break;

        case 1: /* Reset type 1 -> reset to device settings */
            if (kCipStatusError == OpENer_Interface::ResetDeviceToInitialConfiguration().status)
            {
                message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
            }
            break;

        /* case 2: Not supported Reset type 2 -> Return to factory defaults except communications parameters */

        default:
            message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
            break;
        }
    }
    else /*TODO: Should be if (pa_stMRRequest->DataLength == 0)*/
    {
        /* The same behavior as if the data value given would be 0
     emulate device reset */

        if (kCipStatusError == OpENer_Interface::ResetDevice().status)
        {
            message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
        }
        else
        {
            /* eip_status = EIP_OK; */
        }
    }
    message_router_response->response_data->clear ();
    return eip_status;
}

/** @brief CIP Identity object constructor
 *
 */
CipStatus CIP_Identity::Init()
{
    if (number_of_instances == 0)
    {
        // attributes in CIP Identity Object
        vendor_id_     = OPENER_DEVICE_VENDOR_ID;
        device_type_   = OPENER_DEVICE_TYPE;
        product_code_  = OPENER_DEVICE_PRODUCT_CODE;
        revision_      = {OPENER_DEVICE_MAJOR_REVISION, OPENER_DEVICE_MINOR_REVISION};
        status_        = 0;
        serial_number_ = 0;
        product_name_  = {sizeof(OPENER_DEVICE_NAME) - 1, (CipByte *) OPENER_DEVICE_NAME};

        class_id = kCipIdentityClassCode;
        //get_all_class_attributes_mask = MASK4(1, 2, 6, 7);
        //get_all_instance_attributes_mask = MASK7(1, 2, 3, 4, 5, 6, 7);
        class_name = "Identity";
        revision = 1;

        CIP_Identity *instance = new CIP_Identity();
        AddClassInstance(instance, 0);

        instance->classAttributesProperties.emplace(1, CipAttributeProperties_t{kCipUint       , sizeof(CipUint       ), kGetableSingleAndAll, "&vendor_id"     });
        instance->classAttributesProperties.emplace(2, CipAttributeProperties_t{kCipUint       , sizeof(CipUint       ), kGetableSingleAndAll, "&device_type_"  });
        instance->classAttributesProperties.emplace(3, CipAttributeProperties_t{kCipUint       , sizeof(CipUint       ), kGetableSingleAndAll, "&product_code_" });
        instance->classAttributesProperties.emplace(4, CipAttributeProperties_t{kCipUsintUsint , sizeof(CipByteArray  ), kGetableSingleAndAll, "&revision_"     });
        instance->classAttributesProperties.emplace(5, CipAttributeProperties_t{kCipWord       , sizeof(CipWord       ), kGetableSingleAndAll, "&status_"       });
        instance->classAttributesProperties.emplace(6, CipAttributeProperties_t{kCipUdint      , sizeof(CipUdint      ), kGetableSingleAndAll, "&serial_number_"});
        instance->classAttributesProperties.emplace(7, CipAttributeProperties_t{kCipShortString, sizeof(CipShortString), kGetableSingleAndAll, "&product_name_" });
    }
    return kCipGeneralStatusCodeSuccess;
}

CipStatus CIP_Identity::Shut(void)
{

}

void * CIP_Identity::retrieveAttribute(CipUsint attributeNumber)
{
    if (this->id == 0)
    {
        switch(attributeNumber)
        {
            case 1:
                return &this->vendor_id_;
            case 2:
                return &this->device_type_;
            case 3:
                return &this->product_code_;
            case 4:
                return &this->revision_;
            case 5:
                return &this->status_;
            case 6:
                return &this->serial_number_;
            case 7:
                return &this->product_name_;
            default:
                return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

CipStatus CIP_Identity::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{

}