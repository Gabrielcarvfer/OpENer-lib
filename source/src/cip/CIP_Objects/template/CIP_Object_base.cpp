//
// Created by gabriel on 10/12/2017.
//

#include <cip/connection/network/NET_Endianconv.hpp>
#include <CIP_Object_base.hpp>


int CIP_Object_base::EncodeData (CipUsint cip_type, void *data, std::vector<CipUsint> *message)
{
    int counter = 0;

    switch (cip_type)
        /* check the data type of attribute */
    {
        case (kCipBool):
        case (kCipSint):
        case (kCipUsint):
        case (kCipByte):
            message->push_back(* (CipUsint*) data);
            break;

        case (kCipInt):
        case (kCipUint):
        case (kCipWord):
            switch(NET_Endianconv::g_opENer_platform_endianess)
            {
                case NET_Endianconv::kOpENerEndianessLittle:
                    message->push_back(* (CipUsint*) (data)  );
                    message->push_back(* (CipUsint*) (data+4));
                    break;
                case NET_Endianconv::kOpENerEndianessBig:
                    message->push_back(* (CipUsint*) (data+4));
                    message->push_back(* (CipUsint*) (data)  );
                    break;
                default:
                    //todo: Report error
                    break;
            }
            break;

        case (kCipDint):
        case (kCipUdint):
        case (kCipDword):
        case (kCipReal):
            switch(NET_Endianconv::g_opENer_platform_endianess)
            {
                case NET_Endianconv::kOpENerEndianessLittle:
                    message->push_back(* (CipUsint*) (data)   );
                    message->push_back(* (CipUsint*) (data+4) );
                    message->push_back(* (CipUsint*) (data+8) );
                    message->push_back(* (CipUsint*) (data+12));
                    break;
                case NET_Endianconv::kOpENerEndianessBig:
                    message->push_back(* (CipUsint*) (data+12));
                    message->push_back(* (CipUsint*) (data+8) );
                    message->push_back(* (CipUsint*) (data+4) );
                    message->push_back(* (CipUsint*) (data)   );
                    break;
                default:
                    //todo: Report error
                    break;
            }
            break;

        case (kCipLint):
        case (kCipUlint):
        case (kCipLword):
        case (kCipLreal):
            switch(NET_Endianconv::g_opENer_platform_endianess)
            {
                case NET_Endianconv::kOpENerEndianessLittle:
                    message->push_back(* (CipUsint*) (data)   );
                    message->push_back(* (CipUsint*) (data+4) );
                    message->push_back(* (CipUsint*) (data+8) );
                    message->push_back(* (CipUsint*) (data+12));
                    message->push_back(* (CipUsint*) (data+16));
                    message->push_back(* (CipUsint*) (data+20));
                    message->push_back(* (CipUsint*) (data+24));
                    message->push_back(* (CipUsint*) (data+28));
                    break;
                case NET_Endianconv::kOpENerEndianessBig:
                    message->push_back(* (CipUsint*) (data+28));
                    message->push_back(* (CipUsint*) (data+24));
                    message->push_back(* (CipUsint*) (data+20));
                    message->push_back(* (CipUsint*) (data+16));
                    message->push_back(* (CipUsint*) (data+12));
                    message->push_back(* (CipUsint*) (data+8) );
                    message->push_back(* (CipUsint*) (data+4) );
                    message->push_back(* (CipUsint*) (data)   );
                    break;
                default:
                    //todo: Report error
                    break;
            }
/*

        case (kCipStime):
        case (kCipDate):
        case (kCipTimeOfDay):
        case (kCipDateAndTime):
            break;
        case (kCipString):
        {
            auto *string = (CipString *) data;

            NET_Endianconv::AddIntToMessage (* &(string->length), message);
            memcpy (message, string->string, string->length);
            *message += string->length;

            counter = string->length + 2; // we have a two byte length field
            if (counter & 0x01)
            {
                // we have an odd byte count
                *message = 0;
                ++(*message);
                counter++;
            }
            break;
        }
        case (kCipString2):
        case (kCipFtime):
        case (kCipLtime):
        case (kCipItime):
        case (kCipStringN):
            break;

        case (kCipShortString):
        {
            CipShortString *short_string = (CipShortString *) data;

            *message = short_string->length;
            ++(*message);

            memcpy (message, short_string->string, short_string->length);
            *message += short_string->length;

            counter = short_string->length + 1;
            break;
        }

        case (kCipTime):
            break;

        case (kCipEpath):
            counter = EncodeEPath ((CipEpath *) data, message);
            break;

        case (kCipEngUnit):
            break;

        case (kCipUsintUsint):
        {
            identityRevision_t *revision = (identityRevision_t *) data;

            *message = revision->major_revision;
            ++(*message);
            *message = revision->minor_revision;
            ++(*message);
            counter = 2;
            break;
        }

        case (kCipUdintUdintUdintUdintUdintString):
        {
            // TCP/IP attribute 5
            CipTcpIpNetworkInterfaceConfiguration *tcp_ip_network_interface_configuration = (CipTcpIpNetworkInterfaceConfiguration *) data;
            NET_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->ip_address), message);
            NET_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->network_mask), message);
            NET_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->gateway), message);
            NET_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->name_server), message);
            NET_Endianconv::AddDintToMessage (NET_Connection::endian_ntohl (tcp_ip_network_interface_configuration->name_server_2), message);
            counter = 20;
            counter += EncodeData (kCipString, &(tcp_ip_network_interface_configuration->domain_name), message);
            break;
        }

        case (kCip6Usint):
        {
            CipUsint *p = (CipUsint *) data;
            memcpy (message, p, 6);
            counter = 6;
            break;
        }

        case (kCipMemberList):
            break;

        case (kCipByteArray):
        {
            CipByteArray *cip_byte_array;
            OPENER_TRACE_INFO(" -> get attribute byte array\r\n");
            cip_byte_array = (CipByteArray *) data;
            memcpy (message, cip_byte_array->data, cip_byte_array->length);
            *message += cip_byte_array->length;
            counter = cip_byte_array->length;
        }
            break;

        case (kInternalUint6): // TODO for port class attribute 9, hopefully we can find a better way to do this
        {
            CipUint *internal_unit16_6 = (CipUint *) data;

            NET_Endianconv::AddIntToMessage (internal_unit16_6[0], message);
            NET_Endianconv::AddIntToMessage (internal_unit16_6[1], message);
            NET_Endianconv::AddIntToMessage (internal_unit16_6[2], message);
            NET_Endianconv::AddIntToMessage (internal_unit16_6[3], message);
            NET_Endianconv::AddIntToMessage (internal_unit16_6[4], message);
            NET_Endianconv::AddIntToMessage (internal_unit16_6[5], message);
            counter = 12;
            break;
        }
    */
        default:
            break;
    }

    return (int) message->size();
}

int CIP_Object_base::DecodeData (CipUsint cip_type, void *data, CipUsint *message)
{
    int number_of_decoded_bytes = -1;

    switch (cip_type)
        /* check the data type of attribute */
    {
        case (kCipBool):
        case (kCipSint):
        case (kCipUsint):
        case (kCipByte):
            *(CipUsint *) (data) = *message;
            ++(*message);
            number_of_decoded_bytes = 1;
            break;

        case (kCipInt):
        case (kCipUint):
        case (kCipWord):
            (*(CipUint *) (data)) = NET_Endianconv::GetIntFromMessage (message);
            number_of_decoded_bytes = 2;
            break;

        case (kCipDint):
        case (kCipUdint):
        case (kCipDword):
            (*(CipUdint *) (data)) = NET_Endianconv::GetDintFromMessage (message);
            number_of_decoded_bytes = 4;
            break;

        case (kCipLint):
        case (kCipUlint):
        case (kCipLword):
        {
            (*(CipUlint*)(data)) = NET_Endianconv::GetLintFromMessage(message);
            number_of_decoded_bytes = 8;
        }
            break;

        case (kCipString):
        {
            CipString *string = (CipString *) data;
            string->length = NET_Endianconv::GetIntFromMessage (message);
            memcpy (string->string, message, string->length);
            *message += string->length;

            number_of_decoded_bytes = string->length + 2; /* we have a two byte length field */
            if (number_of_decoded_bytes & 0x01)
            {
                /* we have an odd byte count */
                ++(*message);
                number_of_decoded_bytes++;
            }
        }
            break;
        case (kCipShortString):
        {
            CipShortString *short_string = (CipShortString *) data;

            short_string->length = *message;
            ++(*message);

            memcpy (short_string->string, message, short_string->length);
            *message += short_string->length;

            number_of_decoded_bytes = short_string->length + 1;
            break;
        }

        default:
            break;
    }

    return number_of_decoded_bytes;
}