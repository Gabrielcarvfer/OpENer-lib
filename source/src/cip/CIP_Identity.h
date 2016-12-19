/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIPIDENTITY_H_
#define OPENER_CIPIDENTITY_H_

#include <ciptypes.h>
#include <typedefs.h>
#include <CIP_Object.h>
#include <CIP_Object.h>

class CIP_Identity;

class CIP_Identity : public CIP_Object
{
public:
/* global public variables */
/* attributes in CIP Identity Object */

// Attribute 1: Vendor ID
    static CipUint vendor_id_;
// Attribute 2: Device Type
    static CipUint device_type_;
// Attribute 3: Product Code
    static CipUint product_code_;
// Attribute 4: Revision / USINT Major, USINT Minor
    static CipRevision revision_;
// Attribute 5: Status
    static CipUint status_;
// Attribute 6: Serial Number, has to be set prior to OpENer initialization
    static CipUdint serial_number_;
// Attribute 7: Product Name
    static CipShortString product_name_;

/* public functions */
/** @brief CIP Identity object constructor
 *
 * @returns EipError if the class could not be created, otherwise EipOk
 */
    static CipStatus CipIdentityInit (void);
private:
    /** @brief Status of the CIP Identity object */
    typedef enum
    {
        // Indicates that the device has an owner
                kOwned = 0x0001,

        // Indicates that the device is configured to do something different, than the out-of-the-box default.
                kConfigured = 0x0004,

        // Indicates that the device detected a fault with itself, which was thought to be recoverable.
        // The device did not switch to a faulted state.
                kMinorRecoverableFault = 0x0100,

        // Indicates that the device detected a fault with itself, which was thought to be recoverable.
        // The device did not switch to a faulted state.
                kMinorUncoverableFault = 0x0200,

        // Indicates that the device detected a fault with itself,which was thought to be recoverable.
        // The device changed to the "Major Recoverable Fault" state
                kMajorRecoveralbeFault = 0x0400,

        // Indicates that the device detected a fault with itself,which was thought to be recoverable.
        // The device changed to the "Major Unrecoverable Fault" state
                kMajorUnrecoverableFault = 0x0800

    } CipIdentityStatus;

    typedef enum
    {
        kSelftestingUnknown = 0x0000,
        kFirmwareUpdateInProgress = 0x0010,
        kStatusAtLeastOneFaultedIoConnection = 0x0020,
        kNoIoConnectionsEstablished = 0x0030,
        kNonVolatileConfigurationBad = 0x0040,
        kMajorFault = 0x0050,
        kAtLeastOneIoConnectionInRuneMode = 0x0060,
        kAtLeastOneIoConnectionEstablishedAllInIdleMode = 0x0070
    } CipIdentityExtendedStatus;
public:
    CipStatus InstanceServices(int service, CipMessageRouterRequest *msg_router_request, CipMessageRouterResponse *msg_router_response);
    CipStatus Reset(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);
    static void SetDeviceStatus(CipUint status);
    static void SetDeviceSerialNumber(CipUdint serial_number);
};
#endif /* OPENER_CIPIDENTITY_H_ */
