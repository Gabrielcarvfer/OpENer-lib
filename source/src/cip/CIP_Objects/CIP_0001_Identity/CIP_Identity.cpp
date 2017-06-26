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

#include <cip/ciptypes.hpp>
#include "CIP_Identity.hpp"

//Methods

/** Private functions, sets the devices serial number
 * @param serial_number The serial number of the device
 */
void CIP_Identity::SetDeviceSerialNumber(CipUdint serial_number)
{
    serial_number = serial_number;
}

/** Private functions, sets the devices status
 * @param status The serial number of the deivce
 */
void CIP_Identity::SetDeviceStatus(CipUint status)
{
    status = status;
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
            case 0: // Reset type 0 -> emulate device reset / Power cycle /
                //todo: emulate reset if (kCipStatusError == OpENer_Interface::ResetDevice().status)
                {
                    message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
                }
                break;

            case 1: // Reset type 1 -> reset to device settings/
                //todo: try to reset settings if (kCipStatusError == OpENer_Interface::ResetDeviceToInitialConfiguration().status)
                {
                    message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
                }
                break;

            case 2: // Reset type 2 -> Return to factory defaults except communications parameters /
                {}
                break;

            //-----Vendor specific ranging from 100 to 199------

            default: // Reserved by CIP ranging from 3 to 99 and from 200 to 255
                message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
                break;
        }
    }
    else //TODO: Should be if (pa_stMRRequest->DataLength == 0)/
    {
        // The same behavior as if the data value given would be 0 emulate device reset/

        //todo: fix if (kCipStatusError == OpENer_Interface::ResetDevice().status)
        {
            message_router_response->general_status = kCipGeneralStatusCodeInvalidParameter;
        }
        //else
        {
            // eip_status = EIP_OK; 
        }
    }
   message_router_response->response_data.clear ();
    return eip_status;
}

/** @brief CIP Identity object constructor
 *
 */
CipStatus CIP_Identity::Init() {
    CipStatus stat;

    if (number_of_instances == 0) {

        class_id = kCipIdentityClassCode;
        //get_all_class_attributes_mask = MASK4(1, 2, 6, 7);
        //get_all_instance_attributes_mask = MASK7(1, 2, 3, 4, 5, 6, 7);
        class_name = "Identity";
        RegisterGenericClassAttributes();

        CIP_Identity *instance = new CIP_Identity();
        AddClassInstance(instance, 0);

        instance->revision = identityRevision_t {1, 0};

        instAttrInfo.emplace(1, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "vendor_id"});
        instAttrInfo.emplace(2, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "device_type"});
        instAttrInfo.emplace(3, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll, "product_code"});
        instAttrInfo.emplace(4,
                             CipAttrInfo_t{kCipUsintUsint, SZ(identityRevision_t), kAttrFlagGetableSingleAndAll, "revision"});
        instAttrInfo.emplace(5, CipAttrInfo_t{kCipWord, SZ(CipWord), kAttrFlagGetableSingleAndAll, "status"});
        instAttrInfo.emplace(6, CipAttrInfo_t{kCipUdint, SZ(CipUdint), kAttrFlagGetableSingleAndAll, "serial_number"});
        instAttrInfo.emplace(7,
                             CipAttrInfo_t{kCipShortString, SZ(CipShortString), kAttrFlagGetableSingleAndAll, "product_name"});
        instAttrInfo.emplace(8, CipAttrInfo_t{kCipUsint, SZ(CipUsint), kAttrFlagGetableSingleAndAll, "State"});
        instAttrInfo.emplace(9, CipAttrInfo_t{kCipUint, SZ(CipUint), kAttrFlagGetableSingleAndAll,
                                              "Configuration_consistency_value"});
        instAttrInfo.emplace(10, CipAttrInfo_t{kCipUsint, SZ(CipUsint), kAttrFlagGetableSingleAndAll, "Heartbeat Interval"});

        /*  todo:
        instAttrInfo.emplace(11, CipAttrInfo_t{kCipUsint), 3*SZ(CipUsint    ), kAttrFlagSetable            , "Active Language"});
        instAttrInfo.emplace(12, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"Supported Language List"});
        instAttrInfo.emplace(13, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"International Product Name"});
        instAttrInfo.emplace(14, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"Semaphore"});
        instAttrInfo.emplace(15, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"Assigned_Name"});
        instAttrInfo.emplace(16, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"AssignedDescription"});
        instAttrInfo.emplace(17, CipAttrInfo_t{kCip,SZ(),kAttrFlagGetable,"Geographic_Location"});
        instAttrInfo.emplace(18, CipAttrInfo_t{kCip,sizeof(v7.c5.),kNotSetOrGetable,"Modbus Identity Info"});
         */
        stat.status = kCipStatusOk;
    }
    else
    {
        stat.status = kCipStatusError;
    }
    return stat;
}

CipStatus CIP_Identity::Create()
{
	CipStatus stat;
    CIP_Identity *instance = new CIP_Identity();
    AddClassInstance(instance, -1);

    // attributes in CIP Identity Object
    instance->vendor_id     = OPENER_DEVICE_VENDOR_ID;
    instance->device_type   = OPENER_DEVICE_TYPE;
    instance->product_code  = OPENER_DEVICE_PRODUCT_CODE;
    instance->revision      = {OPENER_DEVICE_MAJOR_REVISION, OPENER_DEVICE_MINOR_REVISION};
    //instance->status        =
    instance->serial_number = 0;
    instance->product_name  = {sizeof(OPENER_DEVICE_NAME) - 1, (CipByte *) OPENER_DEVICE_NAME};

	return stat;
}

CipStatus CIP_Identity::Shut()
{
	return kCipStatusOk;
}

void * CIP_Identity::retrieveAttribute(CipUsint attributeNumber)
{
    if (this->id == 0)
    {
        switch(attributeNumber)
        {
            case 1:return &this->vendor_id;
            case 2:return &this->device_type;
            case 3:return &this->product_code;
            case 4:return &this->revision;
            case 5:return &this->status;
            case 6:return &this->serial_number;
            case 7:return &this->product_name;
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
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeServiceNotSupported;
	return stat;
}