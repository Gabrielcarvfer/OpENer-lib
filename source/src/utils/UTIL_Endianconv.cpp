
#include "UTIL_Endianconv.h"
#include <string.h>


/* THESE ROUTINES MODIFY THE BUFFER POINTER*/

/**
 *   @brief Reads EIP_UINT8 from *buffer and converts little endian to host.
 *   @param buffer pointer where data should be read.
 *   @return EIP_UINT8 data value
 */
CipUsint UTIL_Endianconv::GetSintFromMessage(CipUsint** buffer)
{
    unsigned char* buffer_address = (unsigned char*)*buffer;
    CipUint data = buffer_address[0];
    *buffer += 1;
    return data;
}

/* little-endian-to-host unsigned 16 bit*/

/**
 *   @brief Reads EIP_UINT16 from *buffer and converts little endian to host.
 *   @param buffer pointer where data should be reed.
 *   @return EIP_UINT16 data value
 */
CipUint UTIL_Endianconv::GetIntFromMessage(CipUsint** buffer)
{
    unsigned char* buffer_address = (unsigned char*)*buffer;
    CipUint data = buffer_address[0] | buffer_address[1] << 8;
    *buffer += 2;
    return data;
}

/**
 *   @brief Reads EIP_UINT32 from *buffer and converts little endian to host.
 *   @param buffer pointer where data should be reed.
 *   @return EIP_UNÍT32 value
 */
CipUdint UTIL_Endianconv::GetDintFromMessage(CipUsint** buffer)
{
    unsigned char* p = (unsigned char*)*buffer;
    CipUdint data = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
    *buffer += 4;
    return data;
}

/**
 * @brief converts UINT8 data from host to little endian an writes it to buffer.
 * @param data value to be written
 * @param buffer pointer where data should be written.
 */
int UTIL_Endianconv::AddSintToMessage(CipUsint data, CipUsint** buffer)
{
    unsigned char* p = (unsigned char*)*buffer;

    p[0] = (unsigned char)data;
    *buffer += 1;
    return 1;
}

/**
 * @brief converts UINT16 data from host to little endian an writes it to buffer.
 * @param data value to be written
 * @param buffer pointer where data should be written.
 */
int UTIL_Endianconv::AddIntToMessage(CipUint data, CipUsint** buffer)
{
    unsigned char* p = (unsigned char*)*buffer;

    p[0] = (unsigned char)data;
    p[1] = (unsigned char)(data >> 8);
    *buffer += 2;
    return 2;
}

/**
 * @brief Converts UINT32 data from host to little endian and writes it to buffer.
 * @param data value to be written
 * @param buffer pointer where data should be written.
 */
int UTIL_Endianconv::AddDintToMessage(CipUdint data, CipUsint** buffer)
{
    unsigned char* p = (unsigned char*)*buffer;

    p[0] = (unsigned char)data;
    p[1] = (unsigned char)(data >> 8);
    p[2] = (unsigned char)(data >> 16);
    p[3] = (unsigned char)(data >> 24);
    *buffer += 4;

    return 4;
}

#ifdef OPENER_SUPPORT_64BIT_DATATYPES

/**
 *   @brief Reads CipUlint from *pa_buf and converts little endian to host.
 *   @param pa_buf pointer where data should be reed.
 *   @return CipUlint value
 */
CipUlint UTIL_Endianconv::GetLintFromMessage(CipUsint** buffer)
{
    CipUsint* buffer_address = *buffer;
    CipUlint data = ((((CipUlint)buffer_address[0]) << 56)
                         & 0xFF00000000000000LL)
        + ((((CipUlint)buffer_address[1]) << 48) & 0x00FF000000000000LL)
        + ((((CipUlint)buffer_address[2]) << 40) & 0x0000FF0000000000LL)
        + ((((CipUlint)buffer_address[3]) << 32) & 0x000000FF00000000LL)
        + ((((CipUlint)buffer_address[4]) << 24) & 0x00000000FF000000)
        + ((((CipUlint)buffer_address[5]) << 16) & 0x0000000000FF0000)
        + ((((CipUlint)buffer_address[6]) << 8) & 0x000000000000FF00)
        + (((CipUlint)buffer_address[7]) & 0x00000000000000FF);
    (*buffer) += 8;
    return data;
}

/**
 * @brief Converts CipUlint data from host to little endian and writes it to buffer.
 * @param data value to be written
 * @param buffer pointer where data should be written.
 */
int UTIL_Endianconv::AddLintToMessage(CipUlint data, CipUsint** buffer)
{
    CipUsint* buffer_address = *buffer;
    buffer_address[0] = (CipUsint)(data >> 56) & 0xFF;
    buffer_address[1] = (CipUsint)(data >> 48) & 0xFF;
    buffer_address[2] = (CipUsint)(data >> 40) & 0xFF;
    buffer_address[3] = (CipUsint)(data >> 32) & 0xFF;
    buffer_address[4] = (CipUsint)(data >> 24) & 0xFF;
    buffer_address[5] = (CipUsint)(data >> 16) & 0xFF;
    buffer_address[6] = (CipUsint)(data >> 8) & 0xFF;
    buffer_address[7] = (CipUsint)(data)&0xFF;
    (*buffer) += 8;
    return 8;
}

#endif

/**
 * @brief Detects Endianess of the platform and sets global g_nOpENerPlatformEndianess variable accordingly
 *
 * Detects Endianess of the platform and sets global variable g_nOpENerPlatformEndianess accordingly,
 * whereas 0 equals little endian and 1 equals big endian
 */
void UTIL_Endianconv::DetermineEndianess()
{
    g_opener_platform_endianess = kOpenerEndianessUnknown;
    int i = 1;
    char* p = (char*)&i;
    if (p[0] == 1)
    {
        g_opener_platform_endianess = kOpENerEndianessLittle;
    }
    else
    {
        g_opener_platform_endianess = kOpENerEndianessBig;
    }
}

/**
 * @brief Returns global variable g_nOpENerPlatformEndianess, whereas 0 equals little endian and 1 equals big endian
 *
 * @return 0 equals little endian and 1 equals big endian
 */
int UTIL_Endianconv::GetEndianess()
{
    return g_opener_platform_endianess;
}

void UTIL_Endianconv::MoveMessageNOctets(int amount_of_bytes_moved, CipOctet** message_runner)
{
    (*message_runner) += amount_of_bytes_moved;
}

int UTIL_Endianconv::FillNextNMessageOctetsWith(CipOctet value, unsigned int amount_of_bytes_written, CipOctet** message)
{
    memset(*message, value, amount_of_bytes_written);
    return amount_of_bytes_written;
}

int UTIL_Endianconv::FillNextNMessageOctetsWithValueAndMoveToNextPosition(CipOctet value, unsigned int amount_of_filled_bytes, CipOctet** message)
{
    FillNextNMessageOctetsWith(value, amount_of_filled_bytes, message);
    MoveMessageNOctets(amount_of_filled_bytes, message);
    return amount_of_filled_bytes;
}

