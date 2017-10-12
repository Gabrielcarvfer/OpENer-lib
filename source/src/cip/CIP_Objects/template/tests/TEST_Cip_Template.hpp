//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 17/04/2017.
//

#ifndef OPENERMAIN_TEST_CIP_Template_H
#define OPENERMAIN_TEST_CIP_Template_H


#include "cip/CIP_Objects/template/CIP_Object_template.hpp"

class TEST_Cip_Template1 : public CIP_Object_template<TEST_Cip_Template1>
{
public:
	static CipUint vendor_id_;
	static CipUint device_type_;
	static CipUint product_code_;
	static identityRevision_t revision_;
	static CipUint status_;
	static CipUdint serial_number_;
	static CipShortString product_name_;

	CipStatus getVendorId(CipMessageRouterRequest_t * req, CipMessageRouterResponse_t * resp);
	CipStatus getProductCode(CipMessageRouterRequest_t * req, CipMessageRouterResponse_t * resp);

    static CipStatus Init();
    static CipStatus Shut();
    static CipStatus Create();

	void * retrieveAttribute(CipUsint attributeNumber);
	CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
private:
};

class TEST_Cip_Template2 : public CIP_Object_template<TEST_Cip_Template2>
{
public:
	static CipUint product_code_;
	static CipUint revision_;

    static CipStatus Init();
    static CipStatus Shut();
    static CipStatus Create();

	void * retrieveAttribute(CipUsint attributeNumber);
	CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
};


#endif //OPENERMAIN_TEST_CIP_Template_H
