/***********************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 * ( originally from ciptypes.h - Modified by Gabriel Ferreira (@gabrielcarvfer) )
 **********************************************************************************/

#ifndef CIP_ELETRONICKEY_H
#define CIP_ELETRONICKEY_H

#include "ciptypes.hpp"

/** @brief CIP Electronic Key Segment struct
 *
 */
class CIP_EletronicKey
{
public:
    typedef enum {
        segtype_port_segment          = 0,
        segtype_logical_segment       = 1,
        segtype_network_segment       = 2,
        segtype_symbolic_segment      = 3,
        segtype_data_segment          = 4,
        segtype_data_type_constructed = 5,
        segtype_data_type_elementary  = 6,
        segtype_reserved_segment      = 7
    }eletronic_key_segtype_e;

    typedef union
    {
          struct
          {
              CipUsint seg_type:3;   // look at segtype_e
              CipUsint seg_format:5; // 0 if extended string starts in next byte, 1-31 following size in bytes
          };
          CipUsint value;
    }segment_type_t;

    //Instance values
    segment_type_t segment_type;   // Specifies the Segment Type */
    CipUsint key_format;           // Key Format 0-3 reserved, 4 = see Key Format Table,5-255 = Reserved

    typedef union
    {
            struct
            {
                // Depends on key format used, usually Key Format 4 as
                // specified in CIP Specification, Volume 1, contains
                CipUint vendor_id;      // Vendor ID
                CipUint device_type;    // Device Type
                CipUint product_code;   // Product Code

                // Major Revision and Compatibility (Bit 0-6 = Major Revision) Bit 7 = Compatibility
                // Compatibility set to 0 means key values should match, set to 1 means that any key is accepted
                // If set to 1, key values should be different than 0, or it should generate an path segment error
                CipByte major_revision;
                CipUsint minor_revision; // Minor Revision

            };
            CipUsint data[8];
    }key_data_t;

    key_data_t data;

    //Instance methods
    CipStatus validate_key();

    std::vector<CipUsint> get_packed();
    std::vector<CipUsint> get_padded();

};


#endif //CIP_ELETRONICKEY_H
