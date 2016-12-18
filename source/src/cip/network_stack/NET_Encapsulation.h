//
// Created by gabriel on 15/12/2016.
//

#ifndef NET_ENCAPSULATION_H
#define NET_ENCAPSULATION_H

#include <ciptypes.h>

typedef struct encapsulation_data
{
    CipUint command_code;
    CipUint data_length;
    CipUdint session_handle;
    CipUdint status;
    CipOctet sender_context[8]; /**< length of 8, according to the specification */
    CipUdint options;
    CipUsint *communication_buffer_start; /**< Pointer to the communication buffer used for this message */
    CipUsint *current_communication_buffer_position; /**< The current position in the communication buffer during the decoding process */
} EncapsulationData;

typedef struct encapsulation_interface_information
{
    CipUint type_code;
    CipUint length;
    CipUint encapsulation_protocol_version;
    CipUint capability_flags;
    CipSint name_of_service[16];
} EncapsulationInterfaceInformation;



#endif //NET_ENCAPSULATION_H
