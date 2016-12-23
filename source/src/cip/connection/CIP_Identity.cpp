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
#include "../../OpENer_Interface.hpp"

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
CipStatus CIP_Identity::Reset( CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{
    CipStatus eip_status;

    eip_status = kCipStatusOkSend;

    message_router_response->reply_service = (CipUsint) (0x80 | message_router_request->service);
    message_router_response->size_of_additional_status = 0;
    message_router_response->general_status = kCipErrorSuccess;

    if (message_router_request->data_length == 1)
    {
        switch (message_router_request->data[0])
        {
        case 0: /* Reset type 0 -> emulate device reset / Power cycle */
            if (kCipStatusError == OpENer_Interface::ResetDevice())
            {
                message_router_response->general_status = kCipErrorInvalidParameter;
            }
            break;

        case 1: /* Reset type 1 -> reset to device settings */
            if (kCipStatusError == OpENer_Interface::ResetDeviceToInitialConfiguration())
            {
                message_router_response->general_status = kCipErrorInvalidParameter;
            }
            break;

        /* case 2: Not supported Reset type 2 -> Return to factory defaults except communications parameters */

        default:
            message_router_response->general_status = kCipErrorInvalidParameter;
            break;
        }
    }
    else /*TODO: Should be if (pa_stMRRequest->DataLength == 0)*/
    {
        /* The same behavior as if the data value given would be 0
     emulate device reset */

        if (kCipStatusError == OpENer_Interface::ResetDevice())
        {
            message_router_response->general_status = kCipErrorInvalidParameter;
        }
        else
        {
            /* eip_status = EIP_OK; */
        }
    }
    message_router_response->data_length = 0;
    return eip_status;
}

/** @brief CIP Identity object constructor
 *
 * @returns EIP_ERROR if the class could not be created, otherwise EIP_OK
 */
CipStatus CIP_Identity::CipIdentityInit()
{
    // attributes in CIP Identity Object
    vendor_id_ = OPENER_DEVICE_VENDOR_ID;
    device_type_ = OPENER_DEVICE_TYPE;
    product_code_ = OPENER_DEVICE_PRODUCT_CODE;
    revision_ = { OPENER_DEVICE_MAJOR_REVISION,  OPENER_DEVICE_MINOR_REVISION };
    status_ = 0;
    serial_number_ = 0;
    product_name_ = { sizeof(OPENER_DEVICE_NAME) - 1, (CipByte*)OPENER_DEVICE_NAME };

    class_id = kCipIdentityClassCode;
    get_all_class_attributes_mask = MASK4(1,2,6,7);
    get_all_instance_attributes_mask = MASK7(1,2,3,4,5,6,7);
    class_name = "Identity";
    revision = 1;

    CIP_Identity * instance = new CIP_Identity();
    AddClassInstance (instance, 0);

    instance->InsertAttribute(1, kCipUint,        &vendor_id_,     kGetableSingleAndAll);
    instance->InsertAttribute(2, kCipUint,        &device_type_,   kGetableSingleAndAll);
    instance->InsertAttribute(3, kCipUint,        &product_code_,  kGetableSingleAndAll);
    instance->InsertAttribute(4, kCipUsintUsint,  &revision_,      kGetableSingleAndAll);
    instance->InsertAttribute(5, kCipWord,        &status_,        kGetableSingleAndAll);
    instance->InsertAttribute(6, kCipUdint,       &serial_number_, kGetableSingleAndAll);
    instance->InsertAttribute(7, kCipShortString, &product_name_,  kGetableSingleAndAll);

    return kCipStatusOk;
}

CipStatus CIP_Identity::InstanceServices(int service, CipMessageRouterRequest *msg_router_request, CipMessageRouterResponse *msg_router_response)
{
    switch(service)
    {
        case kReset:
            return this->Reset(msg_router_request, msg_router_response);
            break;
        default:
            return kCipStatusError;
            break;
    }
}
