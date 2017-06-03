/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef CIP_CLASSES_IDENTITY_H
#define CIP_CLASSES_IDENTITY_H

#include "../../ciptypes.hpp"
#include "../template/CIP_Object.hpp"

class CIP_Identity;
class CIP_Identity : public CIP_Object<CIP_Identity>
{

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

    } CipIdentityStatus_e;

    typedef enum
    {
        kCipIdentityExtendedStatusSelftestingUnknown                             = 0,
        kCipIdentityExtendedStatusFirmwareUpdateInProgress                       = 1,
        kCipIdentityExtendedStatusStatusAtLeastOneFaultedIoConnection            = 2,
        kCipIdentityExtendedStatusNoIoConnectionsEstablished                     = 3,
        kCipIdentityExtendedStatusNonVolatileConfigurationBad                    = 4,
        kCipIdentityExtendedStatusMajorFault                                     = 5,
        kCipIdentityExtendedStatusAtLeastOneIoConnectionInRuneMode               = 6,
        kCipIdentityExtendedStatusAtLeastOneIoConnectionEstablishedAllInIdleMode = 7
    } CipIdentityExtendedStatus_e;

    typedef struct
    {
        CipUsint language1;
        CipUsint language2;
        CipUsint language3;
    } activeLanguage_t;

    typedef struct
    {
        struct
        {
            CipUint vendorNumber;
            CipUdint clientSerialNumber;
        } clientElectronicKey;

        CipUsint /*ITIME?*/ semaphoreTimer;
    } semaphore_t;

    typedef union
    {
            CipUint val;
            struct
            {
                CipUint owned:1;
                CipUint reserved1:1;
                CipUint configured:1;
                CipUint reserved2:1;
                CipUint extendedDeviceStatus:4;
                CipUint minorRecoverableFault:1;
                CipUint majorRecovarableFault:1;
                CipUint reserved3:3;
            } bitfield_u;
    } identityStatus_t;



    void * retrieveAttribute(CipUsint attributeNumber);
    CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
public:
    CipStatus Reset(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);
    static void SetDeviceStatus(CipUint status);
    static void SetDeviceSerialNumber(CipUdint serial_number);

    // Object instance attributes 1 to 18
    CipUint            vendor_id;
    CipUint            device_type;
    CipUint            product_code;
    identityRevision_t revision;
    identityStatus_t   status;
    CipUint            serial_number;
    CipShortString     product_name;
    CipUsint           state;
    CipUint            configurationConsistencyVal;
    CipUsint           heartbeatInterval;
    activeLanguage_t   activeLanguage;
    std::vector<activeLanguage_t> supporterLanguageList;
    CipString/*stringi?*/ internationalProductName;
    semaphore_t      semaphore;
    CipString/*stringi?*/ assignedName;
    CipString/*stringi?*/ assignedDescription;
    CipString/*stringi?*/ geographicLocation;
    CipString/*Vol.7 Chap. 5*/ modbusIdentityInfo;


/* public functions */
/** @brief CIP Identity object constructor
 *
 * @returns EipError if the class could not be created, otherwise EipOk
 */
    static CipStatus Create(void);
    static CipStatus Init (void);
    static CipStatus Shut (void);
};
#endif /* CIP_CLASSES_IDENTITY_H */
