/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#ifndef OPENER_ENDIANCONV_H_
#define OPENER_ENDIANCONV_H_

#include <typedefs.h>

/** @file endianconv.h
 * @brief Responsible for Endianess conversion
 */
class UTIL_Endianconv
{
public:
    typedef enum
    {
        kOpenerEndianessUnknown = -1, kOpENerEndianessLittle = 0, kOpENerEndianessBig = 1
    } OpenerEndianess;

    static OpenerEndianess g_opener_platform_endianess;

/** @ingroup ENCAP
 *   @brief Reads EIP_UINT8 from *buffer and converts little endian to host.
 *   @param buffer pointer where data should be reed.
 *   @return EIP_UINT8 data value
 */
    static CipUsint GetSintFromMessage (CipUsint **buffer);

/** @ingroup ENCAP
 *
 * @brief Get an 16Bit integer from the network buffer, and moves pointer beyond the 16 bit value
 * @param buffer Pointer to the network buffer array. This pointer will be incremented by 2!
 * @return Extracted 16 bit integer value
 */
    static CipUint GetIntFromMessage (CipUsint **buffer);

/** @ingroup ENCAP
 *
 * @brief Get an 32Bit integer from the network buffer.
 * @param buffer pointer to the network buffer array. This pointer will be incremented by 4!
 * @return Extracted 32 bit integer value
 */
    static CipUdint GetDintFromMessage (CipUsint **buffer);

/** @ingroup ENCAP
 *
 * @brief converts UINT8 data from host to little endian an writes it to buffer.
 * @param data value to be written
 * @param buffer pointer where data should be written.
 */
    static  int AddSintToMessage (CipUsint data, CipUsint **buffer);

/** @ingroup ENCAP
 *
 * @brief Write an 16Bit integer to the network buffer.
 * @param data value to write
 * @param buffer pointer to the network buffer array. This pointer will be incremented by 2!
 *
 * @return Length in bytes of the encoded message
 */
    static  int AddIntToMessage (CipUint data, CipUsint **buffer);

/** @ingroup ENCAP
 *
 * @brief Write an 32Bit integer to the network buffer.
 * @param data value to write
 * @param buffer pointer to the network buffer array. This pointer will be incremented by 4!
 *
 * @return Length in bytes of the encoded message
 */
    static  int AddDintToMessage (CipUdint data, CipUsint **buffer);

#ifdef OPENER_SUPPORT_64BIT_DATATYPES

    CipUlint GetLintFromMessage(CipUsint** buffer);

    /** @ingroup ENCAP
     *
     * @brief Write an 64Bit integer to the network buffer.
     * @param data value to write
     * @param buffer pointer to the network buffer array. This pointer will be incremented by 8!
     *
     * @return Length in bytes of the encoded message
     */
    int AddLintToMessage(CipUlint pa_unData, CipUsint** buffer);

#endif

/** Identify if we are running on a big or little endian system and set
 * variable.
 */
    static  void DetermineEndianess (void);

/** @brief Return the endianess identified on system startup
 * @return
 *    - -1 endianess has not been identified up to now
 *    - 0  little endian system
 *    - 1  big endian system
 */
    static int GetEndianess (void);

    static void MoveMessageNOctets (int n, CipOctet **message_runner);

    static int FillNextNMessageOctetsWith (CipOctet value, unsigned int n, CipOctet **message);

    static int FillNextNMessageOctetsWithValueAndMoveToNextPosition (CipOctet value, unsigned int n, CipOctet **message);


};
#endif /* OPENER_ENDIANCONV_H_ */
