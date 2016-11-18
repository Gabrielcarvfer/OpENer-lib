//
// Created by gabriel on 11/11/2016.
//

#ifndef OPENER_CIP_ATTRIBUTE_H
#define OPENER_CIP_ATTRIBUTE_H

#include "../ciptypes.h"
class CIP_Attribute
{
    public:
        //Constructor and deconstructor
        CIP_Attribute(CipUint attribute_number, CipUsint type, void * data, CipAttributeFlag attributeFlag);
        ~CIP_Attribute ();

        //Functions avaiable
        CipUint getNumber();
        CipUsint getType();
        CipAttributeFlag getFlag();
        void * getData();
    private:
        CipUint attribute_number;
        CipUsint type;
        /*< 0 => getable_all, 1 => getable_single; 2 => setable_single; 3 => get and setable; all other values reserved */
        CipAttributeFlag attribute_flags;
        void* data;
};


#endif //OPENER_CIP_ATTRIBUTE_H
