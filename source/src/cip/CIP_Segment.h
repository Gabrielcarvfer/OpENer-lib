//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 07/03/2017.
//

#ifndef CIP_SEGMENT_H
#define CIP_SEGMENT_H

#include "ciptypes.hpp"

class CIP_Segment
{
    typedef enum {
        segtype_port_segment          = 0, //
        segtype_logical_segment       = 1, // Packed and padded epaths + electronic keys
        segtype_network_segment       = 2, // Specify network parameters to send messages through network
        segtype_symbolic_segment      = 3,
        segtype_data_segment          = 4,
        segtype_data_type_constructed = 5,
        segtype_data_type_elementary  = 6
        // 7 is reserved
    }segment_type_e;

    typedef enum {
        logical_classid         = 0,
        logical_instanceid      = 1,
        logical_memberid        = 2,
        logical_connectionpoint = 3,
        logical_attributeid     = 4,
        logical_special         = 5, // only 8 bit format is valid, others are reserved; used for electronic key
        logical_serviceid       = 6  // only 8 bit format is valid, others are reserved
        // 7 is reserved
    }segment_logical_type_e;

    typedef enum{
        logical_8bitaddress  = 0,
        logical_16bitaddress = 1,
        logical_32bitaddress = 2,
        // 3 is reserved
    }segment_logical_format_e;

    typedef enum{
        //  0 to 15 (0xF) payload size is limited to a single byte
        //  0 is reserved
        network_schedule_segment  = 1, //C-2.1 Vol4
        network_fixed_tag_segment = 2, //C-2.1 Vol4
        network_inhibit_time      = 3, // time in mlliseconds (1 to 255) between successive transmissions; 0 means not inhibit C-1.4.3.3
        //  4 (0x04) to 15 F  is reserved

        // 16 (0x10) to 31 (0x1F) payload may have multiple bytes, defined by first byte of payload
        network_safety_segment = 16,
        // 17 (0x11) to 30 (0x1E) is reserved
        network_extended_segment = 31
    }segment_network_subtype_e;

    typedef enum {
        data_simple_segment = 0,
        //  1 (0x01) to 16 (0x10) is reserved
        data_ansi_extended_segment = 17
        // 17 (0x11) to 31 (0x1F) is reserved
    }segment_data_subtype_e;

    typedef union
    {
        struct
        {
            CipUsint seg_type:3;   // look at segment_type_e
            union
            {
                CipUsint seg_format:5;

                //if port segment
                struct
                {
                    // 0 if link address fits in one byte
                    // 1 if link address is bigger than that,
                    // and the next byte is used to specify size of address
                    CipUsint extended_link_address_size:1;

                    //Port identifier indicate port to leave node
                    // need to use a port number or escape to extended port identifier
                    // 0 is reserved
                    // 1 back-plane port
                    // if 15(0xF), then a 16-bit field is added as extended port identifier
                    CipUsint port_identifier:4;
                }port_u;

                //if logical segment
                struct
                {
                    CipUsint logical_type:3;    // type of data; look at segment_logical_type_e
                    CipUsint logical_format:2;  // size of data; look at segment_logical_format_e
                }logical_u;

                //if network segment
                CipUsint network_subtype:5;     // look at segment_network_e

                //if symbolic segment
                CipUsint symbolic_symbolsize:5; // 0 if extended string starts in next byte, 1-31 following size in bytes

                //if data segment
                CipUsint data_subtype:5;        // look at segment_data_subtype_e
            };
        };
        CipUsint value;
    }segment_header_t;

    //Instance values
    segment_header_t segment_header;    // Specifies the Segment Type
    std::vector<CipUsint> segment_payload; // Contents of segment
};


#endif //CIP_SEGMENT_H
