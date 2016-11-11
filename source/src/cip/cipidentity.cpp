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

#include "cipidentity.h"
#include "src/cip/connection_stack/cipcommon.h"
#include "ciperror.h"
#include "src/cip/connection_stack/cipmessagerouter.h"
#include "endianconv.h"
#include "opener_api.h"
#include "opener_user_conf.h"
#include <string.h>

/* attributes in CIP Identity Object */

CipUint vendor_id_ = OPENER_DEVICE_VENDOR_ID; /**< Attribute 1: Vendor ID */
CipUint device_type_ = OPENER_DEVICE_TYPE; /**< Attribute 2: Device Type */
CipUint product_code_ = OPENER_DEVICE_PRODUCT_CODE; /**< Attribute 3: Product Code */
CipRevision revision_ = { OPENER_DEVICE_MAJOR_REVISION,
    OPENER_DEVICE_MINOR_REVISION }; /**< Attribute 4: Revision / USINT Major, USINT Minor */
CipUint status_ = 0; /**< Attribute 5: Status */
CipUdint serial_number_ = 0; /**< Attribute 6: Serial Number, has to be set prior to OpENer initialization */
CipShortString product_name_ = { sizeof(OPENER_DEVICE_NAME) - 1,
    (EipByte*)OPENER_DEVICE_NAME }; /**< Attribute 7: Product Name */

/** Private functions, sets the devices serial number
 * @param serial_number The serial number of the device
 */
void SetDeviceSerialNumber(CipUdint serial_number)
{
    serial_number_ = serial_number;
}

/** Private functions, sets the devices status
 * @param status The serial number of the deivce
 */
void SetDeviceStatus(CipUint status)
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
static CipStatus Reset(CIP_Class* instance, /* pointer to instance*/
    CipMessageRouterRequest* message_router_request, /* pointer to message router request*/
    CipMessageRouterResponse* message_router_response) /* pointer to message router response*/
{
    CipStatus eip_status;
    (void)instance;

    eip_status = kCipStatusOkSend;

    message_router_response->reply_service = (0x80
        | message_router_request->service);
    message_router_response->size_of_additional_status = 0;
    message_router_response->general_status = kCipErrorSuccess;

    if (message_router_request->data_length == 1) {
        switch (message_router_request->data[0]) {
        case 0: /* Reset type 0 -> emulate device reset / Power cycle */
            if (kCipStatusError == ResetDevice()) {
                message_router_response->general_status = kCipErrorInvalidParameter;
            }
            break;

        case 1: /* Reset type 1 -> reset to device settings */
            if (kCipStatusError == ResetDeviceToInitialConfiguration()) {
                message_router_response->general_status = kCipErrorInvalidParameter;
            }
            break;

        /* case 2: Not supported Reset type 2 -> Return to factory defaults except communications parameters */

        default:
            message_router_response->general_status = kCipErrorInvalidParameter;
            break;
        }
    } else /*TODO: Should be if (pa_stMRRequest->DataLength == 0)*/
    {
        /* The same behavior as if the data value given would be 0
     emulate device reset */

        if (kCipStatusError == ResetDevice()) {
            message_router_response->general_status = kCipErrorInvalidParameter;
        } else {
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
CipStatus CipIdentityInit()
{
    CIP_Class* class_ptr;
    CIP_Class* instance;

    class_ptr = CreateCIPClass(kCipIdentityClassCode, 0, /* # of non-default class attributes */
        MASK4(1, 2, 6, 7), /* class getAttributeAll mask		CIP spec 5-2.3.2 */
        0, /* # of class services*/
        7, /* # of instance attributes*/
        MASK7(1, 2, 3, 4, 5, 6, 7), /* instance getAttributeAll mask	CIP spec 5-2.3.2 */
        1, /* # of instance services*/
        1, /* # of instances*/
        "identity", /* class name (for debug)*/
        1); /* class revision*/

    if (class_ptr == 0)
        return kCipStatusError;

    instance = GetCIPClass(class_ptr, 1);

    InsertAttribute(instance, 1, kCipUint, &vendor_id_, kGetableSingleAndAll);
    InsertAttribute(instance, 2, kCipUint, &device_type_, kGetableSingleAndAll);
    InsertAttribute(instance, 3, kCipUint, &product_code_, kGetableSingleAndAll);
    InsertAttribute(instance, 4, kCipUsintUsint, &revision_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 5, kCipWord, &status_, kGetableSingleAndAll);
    InsertAttribute(instance, 6, kCipUdint, &serial_number_,
        kGetableSingleAndAll);
    InsertAttribute(instance, 7, kCipShortString, &product_name_,
        kGetableSingleAndAll);

    InsertService(class, kReset, &Reset, "Reset");

    return kCipStatusOk;
}
