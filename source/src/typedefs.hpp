/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIP_TYPEDEFS_H_
#define OPENER_CIP_TYPEDEFS_H_

#undef WIN
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define WIN
#endif

#include <cstdint>

/** @file typedefs.h
 Do not use interface types for internal variables, such as "int i;", which is
 commonly used for loop counters or counting things.

 Do not over-constrain data types. Prefer the use of the native "int" and
 "unsigned" types.

 Use char for native character strings.

 Do not use "char" for data buffers - use "unsigned char" instead. Using char
 for data buffers can occasionally blow up in your face rather nastily.
 */

/** @brief Data types as defined in the CIP Specification Vol 1 Appendix C
 */
typedef uint8_t  CipOctet; /**< 8 bit value that indicates particular data type */
typedef uint8_t  CipBool;  /**< Boolean data type */
typedef uint8_t  CipByte;  /**< 8-bit bit string */
typedef uint16_t CipWord;  /**< 16-bit bit string */
typedef uint32_t CipDword; /**< 32-bit bit string */
typedef uint8_t  CipUsint; /**< 8-bit unsigned integer */
typedef uint16_t CipUint;  /**< CipUint 16-bit unsigned integer */
typedef uint32_t CipUdint; /**< CipUdint 32-bit unsigned integer */
typedef int8_t   CipSint;  /**< 8-bit signed integer */
typedef int16_t  CipInt;   /**< 16-bit signed integer */
typedef int32_t  CipDint;  /**< 32-bit signed integer */
typedef float    CipReal;  /**< 32-bit IEEE 754 floating point */
typedef double   CipLreal; /**< 64-bit IEEE 754 floating point */


#ifdef OPENER_SUPPORT_64BIT_DATATYPES
typedef int64_t  EipInt64; /**< 64-bit signed number */
typedef uint64_t CipUlint; /**< 64-bit unsigned number */

typedef int64_t  CipLint;  /**< 64-bit signed integer */
typedef uint64_t CipUlint; /**< 64-bit unsigned integer */
typedef uint64_t CipLword; /**< 64-bit bit string */
#endif /* OPENER_SUPPORT_64BIT_DATATYPES */

/** @brief Constant identifying if a socket descriptor is invalid
 */
static const int kEipInvalidSocket = -1;

typedef unsigned long MilliSeconds;
typedef unsigned long long MicroSeconds;

/**

 The following are generally true regarding return status:
 -1 ... an error occurred
 0 ... success

 Occasionally there is a variation on this:
 -1 ... an error occurred
 0 ..  success and there is no reply to send
 1 ... success and there is a reply to send

 For both of these cases EIP_STATUS is the return type.

 Other return type are:
 -- return pointer to thing, 0 if error (return type is "pointer to thing")
 -- return count of something, -1 if error, (return type is int)

 */

/** @brief EIP stack status enum
 *
 */
typedef enum
{
    kCipStatusOk = 0, /**< Stack is ok */
    kCipStatusSend = 1, /**< Stack is ok, after send */
    kCipStatusError = -1 /**< Stack is in error */
} kCipStatus;

class CipStatus
{
    public:
        CipStatus(CipUsint status=kCipStatusOk, CipDint extended_status=0x00)
        {this->status = status; this->extended_status = extended_status;};
        ~CipStatus() = default;
        CipUsint status;
        CipDint extended_status;
};

/** @brief Communication direction of an UDP socket; consuming is receiver,
 * producing is sender
 *
 * These are used as array indexes, watch out if changing these values
 */
typedef enum
{
    kUdpCommuncationDirectionConsuming = 0, /**< Consuming direction; receiver */
    kUdpCommuncationDirectionProducing = 1 /**< Producing direction; sender */
} UdpCommuncationDirection;

#ifndef __cplusplus
/** @brief If we don't have C++ define a C++ -like "bool" keyword defines
 */
typedef enum {
    false = 0, /**< defines "false" as 0 */
    true = 1 /**< defines "true" as 1 */
} BoolKeywords;
#endif /* __cplusplus */

#endif