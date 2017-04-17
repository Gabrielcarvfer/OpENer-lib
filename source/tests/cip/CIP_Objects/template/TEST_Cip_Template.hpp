//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 17/04/2017.
//

#ifndef OPENERMAIN_TEST_CIP_Template_H
#define OPENERMAIN_TEST_CIP_Template_H


#include "../../../../src/cip/CIP_Objects/template/CIP_Object.hpp"

class TEST_Cip_Template1 : public CIP_Object<TEST_Cip_Template1>
{
public:
	static CipUint vendor_id_;
	static CipUint device_type_;
	static CipUint product_code_;
	static CipRevision revision_;
	static CipUint status_;
	static CipUdint serial_number_;
	static CipShortString product_name_;

    static CipStatus Init();
    static CipStatus Shut();
    static CipStatus Create();
};

class TEST_Cip_Template2 : public CIP_Object<TEST_Cip_Template2>
{
public:
	static CipUint product_code_;
	static CipUint revision_;

    static CipStatus Init();
    static CipStatus Shut();
    static CipStatus Create();
};


#endif //OPENERMAIN_TEST_CIP_Template_H
