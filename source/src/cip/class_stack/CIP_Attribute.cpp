//
// Created by gabriel on 11/11/2016.
//

#include <CIP_Attribute.h>

CIP_Attribute::CIP_Attribute(CipUint attribute_number, CipUsint type, void * data, CipAttributeFlag attributeFlag)
{
    // fill in attribute number
    this->attribute_number = attribute_number;

    // fill in attribute type
    this->type = type;

    // fill in attribute flags
    this->attribute_flags = attributeFlag;

    // fill in attribute data
    this->data = data;
}
CIP_Attribute::~CIP_Attribute ()
{

}

CipUint CIP_Attribute::getNumber()
{
    return this->attribute_number;
}

CipUsint CIP_Attribute::getType()
{
    return this->type;
}

CipAttributeFlag CIP_Attribute::getFlag()
{
    return this->attribute_flags;
}

//TODO: fix permissions of flags
void * CIP_Attribute::getData()
{
    return this->data;
}