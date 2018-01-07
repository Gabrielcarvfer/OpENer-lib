/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_CIPERROR_H_
#define OPENER_CIPERROR_H_

#include <typedefs.hpp>

typedef enum 
{
    kCipGeneralStatusCodeSuccess                        = 0x00, /**< Service was successfully performed by the object specified. */
    kCipGeneralStatusCodeConnectionFailure              = 0x01, /**< A connection related service failed along the connection path. */
    kCipGeneralStatusCodeResourceUnavailable            = 0x02, /**< Resources needed for the object to perform the requested service were unavailable */
    kCipGeneralStatusCodeInvalidParameterValue          = 0x03, /**< See Status Code 0x20, which is the preferred value to use for this condition. */
    kCipGeneralStatusCodePathSegmentError               = 0x04, /**< The path segment identifier or the segment syntax was not understood by the processing node. Path processing shall stop when a path segment error is encountered. */
    kCipGeneralStatusCodePathDestinationUnknown         = 0x05, /**< The path is referencing an object class, instance or structure element that is not known or is not contained in the processing node. Path processing shall stop when a path destination unknown error is encountered. */
    kCipGeneralStatusCodePartialTransfer                = 0x06, /**< Only part of the expected data was transferred. */
    kCipGeneralStatusCodeConnectionLost                 = 0x07, /**< The messaging connection was lost. */
    kCipGeneralStatusCodeServiceNotSupported            = 0x08, /**< The requested service was not implemented or was not defined for this Object Class/Instance. */
    kCipGeneralStatusCodeInvalidAttributeValue          = 0x09, /**< Invalid attribute data detected */
    kCipGeneralStatusCodeAttributeListError             = 0x0A, /**< An attribute in the Get_Attribute_List or Set_Attribute_List response has a non-zero status. */
    kCipGeneralStatusCodeAlreadyInRequestedMode         = 0x0B, /**< The object is already in the mode/state being requested by the service */
    kCipGeneralStatusCodeObjectStateConflict            = 0x0C, /**< The object cannot perform the requested service in its current mode/state */
    kCipGeneralStatusCodeObjectAlreadyExists            = 0x0D, /**< The requested instance of object to be created already exists.*/
    kCipGeneralStatusCodeAttributeNotSetable            = 0x0E, /**< A request to modify a non-modifiable attribute was received. */
    kCipGeneralStatusCodePrivilegeViolation             = 0x0F, /**< A permission/privilege check failed */
    kCipGeneralStatusCodeDeviceStateConflict            = 0x10, /**< The device's current mode/state prohibits the execution of the requested service. */
    kCipGeneralStatusCodeReplyDataTooLarge              = 0x11, /**< The data to be transmitted in the response buffer is larger than the allocated response buffer */
    kCipGeneralStatusCodeFragmentationOfAPrimitiveValue = 0x12, /**< The service specified an operation that is going to fragment a primitive data value, i.e. half a REAL data type. */
    kCipGeneralStatusCodeNotEnoughData                  = 0x13, /**< The service did not supply enough data to perform the specified operation. */
    kCipGeneralStatusCodeAttributeNotSupported          = 0x14, /**< The attribute specified in the request is not supported */
    kCipGeneralStatusCodeTooMuchData                    = 0x15, /**< The service supplied more data than was expected */
    kCipGeneralStatusCodeObjectDoesNotExist             = 0x16, /**< The object specified does not exist in the device. */
    kCipGeneralStatusCodeServiceFragmentationSequenceNotInProgress = 0x17, /**< The fragmentation sequence for this service is not currently active for this data. */
    kCipGeneralStatusCodeNoStoredAttributeData          = 0x18, /**< The attribute data of this object was not saved prior to the requested service. */
    kCipGeneralStatusCodeStoreOperationFailure          = 0x19, /**< The attribute data of this object was not saved due to a failure during the attempt. */
    kCipGeneralStatusCodeRoutingFailureRequestPacketTooLarge  = 0x1A, /**< The service request packet was too large for transmission on a network in the path to the destination. The routing device was forced to abort the service. */
    kCipGeneralStatusCodeRoutingFailureResponsePacketTooLarge = 0x1B, /**< The service response packet was too large for transmission on a network in the path from the destination. The routing device was forced to abort the service. */
    kCipGeneralStatusCodeMissingAttributeListEntry      = 0x1C, /**< The service did not supply an attribute in a list of attributes that was needed by the service to perform the requested behavior. */
    kCipGeneralStatusCodeInvalidAttributeValueList      = 0x1D, /**< The service is returning the list of attributes supplied with status information for those attributes that were invalid. */
    kCipGeneralStatusCodeEmbeddedServiceError           = 0x1E, /**< An embedded service resulted in an error. */
    kCipGeneralStatusCodeVendorSpecificError            = 0x1F, /**< A vendor specific error has been encountered. The Additional Code Field of the Error Response defines the particular error encountered. Use of this General Error Code should only be performed when none of the Error Codes presented in this table or within an Object Class definition accurately reflect the error. */
    kCipGeneralStatusCodeInvalidParameter               = 0x20, /**< A parameter associated with the request was invalid. This code is used when a parameter does not meet the requirements of this specification and/or the requirements defined in an Application Object Specification. */
    kCipGeneralStatusCodeWriteonceValueOrMediumAlreadyWritten = 0x21, /**< An attempt was made to write to a write-once medium (e.g. WORM drive, PROM) that has already been written, or to modify a value that cannot be changed once established. */
    kCipGeneralStatusCodeInvalidReplyReceived           = 0x22, /**< An invalid reply is received (e.g. reply service code does not match the request service code, or reply message is shorter than the minimum expected reply size). This status code can serve for other causes of invalid replies. */
    /* 23-24 Reserved by CIP for future extensions */
    kCipGeneralStatusCodeKeyFailureInPath               = 0x25, /**< The Key Segment that was included as the first segment in the path does not match the destination module. The object specific status shall indicate which part of the key check failed. */
    kCipGeneralStatusCodePathSizeInvalid                = 0x26, /**< The size of the path which was sent with the Service Request is either not large enough to allow the Request to be routed to an object or too much routing data was included. */
    kCipGeneralStatusCodeUnexpectedAttributeInList      = 0x27, /**< An attempt was made to set an attribute that is not able to be set at this time. */
    kCipGeneralStatusCodeInvalidMemberId                = 0x28, /**< The Member ID specified in the request does not exist in the specified Class/Instance/Attribute */
    kCipGeneralStatusCodeMemberNotSetable               = 0x29, /**< A request to modify a non-modifiable member was received */
    kCipGeneralStatusCodeGroup2OnlyServerGeneralFailure = 0x2A /**< This error code may only be reported by DeviceNet group 2 only servers with 4K or less code space and only in place of Service not supported, Attribute not supported and Attribute not setable. */
    /*2B - CF Reserved by CIP for future extensions D0 - FF Reserved for Object Class and service errors*/
} kCipGeneralStatusCode;

typedef enum
{
    //For General Status code 0x02
    kCipExtendedStatusCode_OneOrBothConnectionInstancesNonExistent       = 0x01,
    kCipExtendedStatusCode_ConnectionClassOrInstanceOutOfResourcesToBind = 0x02,

} kCipExtendedStatusCode;
#endif /* OPENER_CIPERROR_H_ */
