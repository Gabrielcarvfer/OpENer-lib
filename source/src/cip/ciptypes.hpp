/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#ifndef CIP_CIPTYPES_H
#define CIP_CIPTYPES_H

#include "../typedefs.hpp"
#include "ciperror.hpp"
#include <string>
#include <vector>

/** @brief Segment type Enum
 *
 * Bits 7-5 in the Segment Type/Format byte
 *
 */
typedef enum
{
    /* Segments */
    kSegmentTypePortSegment         = 0x00, // Port segment */
    kSegmentTypeLogicalSegment      = 0x20, // Logical segment */
    kSegmentTypeNetworkSegment      = 0x40, // Network segment */
    kSegmentTypeSymbolicSegment     = 0x60, // Symbolic segment */
    kSegmentTypeDataSegment         = 0x80, // Data segment */
    kSegmentTypeDataTypeConstructed = 0xA0, // Data type constructed */
    kSegmentTypeDataTypeElementary  = 0xC0, // Data type elementary */
    kSegmentTypeSegmentTypeReserved = 0xE0
} SegmentType;

/** @brief Port Segment flags */
typedef enum
{
    kPortSegmentFlagExtendedLinkAddressSize = 0x10 // Extended Link Address Size flag, Port segment */
} PortSegmentFlag;

/** @brief Enum containing values which kind of logical segment is encoded */
typedef enum
{
    kLogicalSegmentLogicalTypeClassId         = 0x00, // Class ID */
    kLogicalSegmentLogicalTypeInstanceId      = 0x04, // Instance ID */
    kLogicalSegmentLogicalTypeMemberId        = 0x08, // Member ID */
    kLogicalSegmentLogicalTypeConnectionPoint = 0x0C, // Connection Point */
    kLogicalSegmentLogicalTypeAttributeId     = 0x10, // Attribute ID */
    kLogicalSegmentLogicalTypeSpecial         = 0x14, // Special */
    kLogicalSegmentLogicalTypeService         = 0x18, // Service ID */
    kLogicalSegmentLogicalTypeExtendedLogical = 0x1C // Extended Logical */
} LogicalSegmentLogicalType;

/** @brief Enum containing values how long the encoded value will be (8, 16, or
 * 32 bit) */
typedef enum
{
    kLogicalSegmentLogicalFormatEightBitValue     = 0x00,
    kLogicalSegmentLogicalFormatSixteenBitValue   = 0x01,
    kLogicalSegmentLogicalFormatThirtyTwoBitValue = 0x02
} LogicalSegmentLogicalFormat;

typedef enum
{
    kProductionTimeInhibitTimeNetworkSegment = 0x43 // identifier indicating a production inhibit time network segment */
} NetworkSegmentSubType;

typedef enum
{
    kDataSegmentTypeSimpleDataMessage         = kSegmentTypeDataSegment + 0x00,
    kDataSegmentTypeAnsiExtendedSymbolMessage = kSegmentTypeDataSegment + 0x11
} DataSegmentType;

/** @brief Enum containing the encoding values for CIP data types for CIP
 * Messages */
typedef enum cip_data_types
{
    kCipAny         = 0x00, // data type that can not be directly encoded */
    kCipBool        = 0xC1, // boolean data type */
    kCipSint        = 0xC2, // 8-bit signed integer */
    kCipInt         = 0xC3, // 16-bit signed integer */
    kCipDint        = 0xC4, // 32-bit signed integer */
    kCipLint        = 0xC5, // 64-bit signed integer */
    kCipUsint       = 0xC6, // 8-bit unsigned integer */
    kCipUint        = 0xC7, // 16-bit unsigned integer */
    kCipUdint       = 0xC8, // 32-bit unsigned integer */
    kCipUlint       = 0xC9, // 64-bit unsigned integer */
    kCipReal        = 0xCA, // Single precision floating point */
    kCipLreal       = 0xCB, // Double precision floating point*/
    kCipStime       = 0xCC, // Synchronous time information*, type of DINT */
    kCipDate        = 0xCD, // Date only*/
    kCipTimeOfDay   = 0xCE, // Time of day */
    kCipDateAndTime = 0xCF, // Date and time of day */
    kCipString      = 0xD0, // Character string, 1 byte per character */
    kCipByte        = 0xD1, // 8-bit bit string */
    kCipWord        = 0xD2, // 16-bit bit string */
    kCipDword       = 0xD3, // 32-bit bit string */
    kCipLword       = 0xD4, // 64-bit bit string */
    kCipString2     = 0xD5, // Character string, 2 byte per character */
    kCipFtime       = 0xD6, // Duration in micro-seconds, high resolution; range of DINT */
    kCipLtime       = 0xD7, // Duration in micro-seconds, high resolution, range of LINT */
    kCipItime       = 0xD8, // Duration in milli-seconds, short; range of INT*/
    kCipStringN     = 0xD9, // Character string, N byte per character */
    kCipShortString = 0xDA, // Character string, 1 byte per character, 1 byte length indicator
    kCipTime        = 0xDB, // Duration in milli-seconds; range of DINT */
    kCipEpath       = 0xDC, // CIP path segments*/
    kCipEngUnit     = 0xDD, // Engineering Units*/
    /* definition of some CIP structs */
    /* need to be validated in IEC 61131-3 subclause 2.3.3 */
    /* TODO: Check these codes */
    kCipUsintUsint = 0xA0, // Used for CIP Identity attribute 4 Revision*/
    kCipUdintUdintUdintUdintUdintString = 0xA1, // TCP/IP attribute 5 - IP address, subnet mask, gateway, IP name server 1, IP name server 2, domain name
    kCip6Usint      = 0xA2, // Struct for MAC Address (six USINTs)*/
    kCipMemberList  = 0xA3, // */
    kCipByteArray   = 0xA4, // */
    kInternalUint6  = 0xF0 // bogus hack, for port class attribute 9, TODO figure out the right way to handle it
} CipDataType;

/** @brief Definition of CIP service codes
 *
 * An Enum with all CIP service codes. Common services codes range from 0x01 to
 *0x1C
 *
 */
typedef enum
{
    /* Start CIP common services */
    kGetAttributeAll        = 0x01,
    kSetAttributeAll        = 0x02,
    kGetAttributeList       = 0x03,
    kSetAttributeList       = 0x04,
    kReset                  = 0x05,
    kStart                  = 0x06,
    kStop                   = 0x07,
    kCreate                 = 0x08,
    kDelete                 = 0x09,
    kMultipleServicePacket  = 0x0A,
    kApplyAttributes        = 0x0D,
    kGetAttributeSingle     = 0x0E,
    kSetAttributeSingle     = 0x10,
    kFindNextObjectInstance = 0x11,
    kRestore                = 0x15,
    kSave                   = 0x16,
    kNoOperation            = 0x17,
    kGetMember              = 0x18,
    kSetMember              = 0x19,
    kInsertMember           = 0x1A,
    kRemoveMember           = 0x1B,
    kGroupSync              = 0x1C,
    /* End CIP common services */

    /* Start CIP object-specific services */
    kForwardOpen            = 0x54,
    kForwardClose           = 0x4E,
    kUnconnectedSend        = 0x52,
    kGetConnectionOwner     = 0x5A
    /* End CIP object-specific services */
} CipServiceCode;

/** @brief Definition of Get and Set Flags for CIP Attributes */
typedef enum
{ /* TODO: Rework */
    kNotSetOrGetable = 0x00, // Neither set-able nor get-able */
    kGetableAll      = 0x01, // Get-able, also part of Get Attribute All service */
    kGetableSingle   = 0x02, // Get-able via Get Attribute */
    kSetable         = 0x04, // Set-able via Set Attribute */
    /* combined for convenience */
    kSetAndGetAble   = 0x07, // both set and get-able */
    kGetableSingleAndAll = 0x03 // both single and all */
} CipAttributeFlag;

typedef enum
{
    kIoConnectionEventOpened,
    kIoConnectionEventTimedOut,
    kIoConnectionEventClosed
} IoConnectionEvent;

/** @brief CIP Byte Array
 *
 */
typedef struct
{
    CipUint length; // Length of the Byte Array
    CipByte* data;  // Pointer to the data
} CipByteArray;

/** @brief CIP Short String
 *
 */
typedef struct
{
    CipUsint length; // Length of the String (8 bit value)
    CipByte* string; // Pointer to the string data
} CipShortString;

/** @brief CIP String
 *
 */
typedef struct
{
    CipUint length; // Length of the String (16 bit value) */
    CipByte* string; // Pointer to the string data */
} CipString;

/** @brief Class for padded EPATHs
 *
 */
class CipEpath
{
public:
    CipUsint path_size;       // Size of the Path in 16-bit words TODO: Fix, should be UINT(EIP_UINT16)
    CipUint class_id;         // Class ID of the linked object */
    CipUint instance_number;  // Requested Instance Number of the linked object */
    CipUint attribute_number; // Requested Attribute Number of the linked object */

    static bool check_if_equal(CipEpath* path0, CipEpath* path1)
    {
        if (path0->path_size != path1->path_size)
            return false;

        if (path0->class_id != path1-> class_id)
            return false;

        if (path0->instance_number != path1->instance_number)
            return false;

        if (path0->attribute_number != path1->attribute_number)
            return false;

        return true;
    }
};

/** @brief CIP Connection Path
 *
 */
typedef struct
{
    CipUsint path_size;
    // Size of the Path in 16-bit words */ /* TODO: Fix, should be UINT(EIP_UINT16)
    CipUdint class_id; // Class ID of the linked object */
    CipUdint connection_point[3]; /* TODO:  Why array length 3? */
    CipUsint data_segment;
    CipUsint* segment_data;
} CipConnectionPath;



typedef struct
{
    CipUsint major_revision;
    CipUsint minor_revision;
} CipRevision;



/** @brief CIP Message Router Request
 *
 */
typedef struct
{
    CipUsint service;
    CipUsint request_path_size;
    CipEpath request_path;
    std::vector<CipUsint> request_data;
} CipMessageRouterRequest_t;

/** @brief CIP Message Router Response
 *
 */
typedef struct
{
    CipUsint  reply_service;              // Reply service code, the requested service code + 0x80
    CipOctet  reserved;                   // Reserved; Shall be zero
    CipUsint  general_status;             // One of the General Status codes listed in CIP Specification Volume 1, Appendix B
    CipUsint  size_additional_status;     // Number of additional 16 bit words in Additional Status Array
    CipUint*  additional_status;          // Array of 16 bit words; If SizeOfAdditionalStatus is 0. there is no Additional Status
    std::vector<CipUsint> *response_data; // Array of octet; Response data per object definition from request
} CipMessageRouterResponse_t;



/** @ingroup CIP_API
 *  @typedef  EIP_STATUS (*TCIPServiceFunc)(S_CIP_Instance *pa_pstInstance,
 *S_CIP_MR_Request *pa_MRRequest, S_CIP_MR_Response *pa_MRResponse)
 *  @brief Signature definition for the implementation of CIP services.
 *
 *  CIP services have to follow this signature in order to be handled correctly
 *by the stack.
 *  @param pa_pstInstance the instance which was referenced in the service
 *request
 *  @param pa_MRRequest request data
 *  @param pa_MRResponse storage for the response data, including a buffer for
 *extended data
 *  @return EIP_OK_SEND if service could be executed successfully and a response
 *should be sent
 */

/**
 * @brief Struct for saving TCP/IP interface information
 */
typedef struct
{
    CipUdint ip_address;
    CipUdint network_mask;
    CipUdint gateway;
    CipUdint name_server;
    CipUdint name_server_2;
    CipString domain_name;
} CipTcpIpNetworkInterfaceConfiguration;

/**
* @brief Struct for saving DeviceNET interface information
*/
typedef struct
{
} CipDeviceNetInterfaceConfiguration;

typedef struct
{
    CipUsint path_size;
    CipUdint port; /* support up to 32 bit path*/
    CipUdint address;
} CipRoutePath;

typedef struct
{
    CipByte priority;
    CipUsint timeout_ticks;
    CipUint message_request_size;
    CipMessageRouterRequest_t message_request;
    CipMessageRouterResponse_t* message_response;
    CipUsint reserved;
    CipRoutePath route_path;
    void* data;
} CipUnconnectedSendParameter;

typedef enum
{
    kCipIdentityClassCode          = 0x01,
    kCipMessageRouterClassCode     = 0x02,
    kCipDeviceNetLinkClassCode     = 0x03,
    kCipAssemblyClassCode          = 0x04,       //Assembly Class Code
    kCipConnectionManagerClassCode = 0x06,
    kCipTcpIpInterfaceClassCode    = 0xF5, // TCP/IP Interface Object class code
    kCipEthernetLinkClassCode      = 0xF6,

} CipClassCodes;

typedef enum
{
    kCipNotificationDuplicate,
    kCipNotificationReceived,
    kCipNotificationSent,
    kCipNotificationFailed
} CipNotification;

#define SET_BIT_N(n)       ((0x01)<< n)
#define X_SET_BIT_N(n,x)   (SET_BIT_N(n)|x)
#define X_RESET_BIT_N(n,x) ((!SET_BIT_N(n))&x)

/* these are used for creating the getAttributeAll masks
 TODO there might be a way simplifying this using __VARARGS__ in #define */
#define MASK1(a) (1 << (a))
#define MASK2(a, b) (1 << (a) | 1 << (b))
#define MASK3(a, b, c) (1 << (a) | 1 << (b) | 1 << (c))
#define MASK4(a, b, c, d) (1 << (a) | 1 << (b) | 1 << (c) | 1 << (d))
#define MASK5(a, b, c, d, e) \
    (1 << (a) | 1 << (b) | 1 << (c) | 1 << (d) | 1 << (e))
#define MASK6(a, b, c, d, e, f) \
    (1 << (a) | 1 << (b) | 1 << (c) | 1 << (d) | 1 << (e) | 1 << (f))
#define MASK7(a, b, c, d, e, f, g) \
    (1 << (a) | 1 << (b) | 1 << (c) | 1 << (d) | 1 << (e) | 1 << (f) | 1 << (g))
#define MASK8(a, b, c, d, e, f, g, h) \
    (1 << (a) | 1 << (b) | 1 << (c) | 1 << (d) | 1 << (e) | 1 << (f) | 1 << (g) | 1 << (h))

#endif