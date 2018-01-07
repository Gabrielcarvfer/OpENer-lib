/***********************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 * ( originally from ciptypes.h - Modified by Gabriel Ferreira (@gabrielcarvfer) )
 **********************************************************************************/

#ifndef CIP_ELECTRONICKEY_H
#define CIP_ELECTRONICKEY_H

#include <ciptypes.hpp>

/** @brief CIP Electronic Key Segment struct
 *
 */
class CIP_ElectronicKey
{
public:
    CIP_ElectronicKey() = default;
    ~CIP_ElectronicKey() = default;

    CipUsint key_format;           // Key Format 0-3 reserved, 4 = see Key Format Table,5-255 = Reserved

    typedef union
    {

        CipUsint data[8];
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
            CipByte  major_revision;
            CipUsint minor_revision; // Minor Revision

        }fields_t;
    }key_data_t;

    key_data_t data;

    //Instance methods
    CipStatus validate_key();


};


#endif //CIP_ELECTRONICKEY_H
