//
// Created by Gabriel Ferreira (@gabrielcarvfer)  on 17/04/2017.
//

#include "TEST_Cip_Template.hpp"

#include <functional>

CipUint TEST_Cip_Template1::vendor_id_;
CipUint TEST_Cip_Template1::device_type_;
CipUint TEST_Cip_Template1::product_code_;
CipRevision TEST_Cip_Template1::revision_;
CipUint TEST_Cip_Template1::status_;
CipUdint TEST_Cip_Template1::serial_number_;
CipShortString TEST_Cip_Template1::product_name_;

CipUint TEST_Cip_Template2::product_code_;
CipUint TEST_Cip_Template2::revision_;

CipStatus TEST_Cip_Template1::getVendorId(CipMessageRouterRequest_t * req, CipMessageRouterResponse_t * resp)
{
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeSuccess;
	stat.extended_status = this->vendor_id_;
	return stat;
}

CipStatus TEST_Cip_Template1::getProductCode(CipMessageRouterRequest_t * req, CipMessageRouterResponse_t * resp)
{
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeSuccess;
	stat.extended_status = this->product_code_;
	return stat;
}
void * TEST_Cip_Template1::retrieveAttribute(CipUsint attributeNumber)
{
	void * attrPtr = nullptr;
	switch (attributeNumber)
	{
		case 1: attrPtr = &vendor_id_;	   break;
		case 2: attrPtr = &device_type_;   break;
		case 3: attrPtr = &product_code_;  break;
		case 4: attrPtr = &revision_;	   break;
		case 5: attrPtr = &status_;		   break;
		case 6: attrPtr = &serial_number_; break;
		case 7: attrPtr = &product_name_;  break;
	default:
		break;
	}
	return attrPtr;
}

CipStatus TEST_Cip_Template1::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeServiceNotSupported;
	stat.extended_status = 0;

	//Class services
	if (this->id == 0)
		switch (serviceNumber)
		{
			case 1: return this->getVendorId(req, resp);
			case 2: return this->getProductCode(req, resp);

			default:
				break;
		}
	//Instance services

	return stat;
}

CipStatus TEST_Cip_Template2::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{
	CipStatus stat;
	stat.status = kCipGeneralStatusCodeServiceNotSupported;
	stat.extended_status = 0;
	return stat;
}
CipStatus TEST_Cip_Template1::Init()
{
	CipStatus stat;
	stat.extended_status = -1;

	if (number_of_instances == 0)
	{
		// attributes in CIP Identity Object
		vendor_id_ = OPENER_DEVICE_VENDOR_ID;
		device_type_ = OPENER_DEVICE_TYPE;
		product_code_ = OPENER_DEVICE_PRODUCT_CODE;
		revision_ = { OPENER_DEVICE_MAJOR_REVISION, OPENER_DEVICE_MINOR_REVISION };
		status_ = 0;
		serial_number_ = 0;
		product_name_ = { sizeof(OPENER_DEVICE_NAME) - 1, (CipByte *)OPENER_DEVICE_NAME };

		class_id = 1;
		class_name = "Temp1";
		revision = 1;

		TEST_Cip_Template1 *instance = new TEST_Cip_Template1();
		AddClassInstance(instance, 0);
		auto f = std::bind(&TEST_Cip_Template1::getProductCode, instance, std::placeholders::_1, std::placeholders::_2);
		instance->classAttributesProperties.emplace(1, CipAttributeProperties_t{ kCipUint, sizeof(kCipUint), kGetableSingleAndAll, "Vendor ID" });
		instance->classAttributesProperties.emplace(2, CipAttributeProperties_t{ kCipUint, sizeof(kCipUint), kGetableSingleAndAll, "Product Code" });
		instance->classServicesProperties.emplace(1, CipServiceProperties_t{ "GetVendorID" });
		instance->classServicesProperties.emplace(2, CipServiceProperties_t{"GetProductCode"});

		instance->InstanceServices(2, nullptr, nullptr);

		CipUint * ptr = (CipUint*)instance->GetCipAttribute(2);

		stat.status = kCipGeneralStatusCodeSuccess;
		stat.extended_status = 0;
	}
	else
	{
		stat.status = kCipGeneralStatusCodeObjectAlreadyExists;
	}
	return stat;
}

void * TEST_Cip_Template2::retrieveAttribute(CipUsint attributeNumber)
{
	void * attrPtr = nullptr;
	switch (attributeNumber)
	{
	case 3: attrPtr = &product_code_;  break;
	case 4: attrPtr = &revision_;	   break;
	default:
		break;
	}
	return attrPtr;
}

CipStatus TEST_Cip_Template2::Init()
{
	CipStatus stat;
	stat.extended_status = -1;

	if (number_of_instances == 0)
	{
		// attributes in CIP Identity Object
		product_code_ = OPENER_DEVICE_PRODUCT_CODE;
		revision_ = 1;

		class_id = 2;
		class_name = "Temp2";
		revision = 1;

		TEST_Cip_Template2 *instance = new TEST_Cip_Template2();
		AddClassInstance(instance, 0);

		//instance->InsertAttribute(1, kCipUint, &product_code_, kGetableSingleAndAll);
		//instance->InsertAttribute(2, kCipUsintUsint, &revision_, kGetableSingleAndAll);

		stat.status = kCipGeneralStatusCodeSuccess;
		stat.extended_status = 0;
	}
	else
	{
		stat.status = kCipGeneralStatusCodeObjectAlreadyExists;
	}

	return stat;
}

CipStatus TEST_Cip_Template1::Shut()
{
	CipStatus stat;

	stat.status = kCipGeneralStatusCodeSuccess;
	return stat;
}

CipStatus TEST_Cip_Template2::Shut()
{
	CipStatus stat;

	stat.status = kCipGeneralStatusCodeSuccess;
	return stat;
}

CipStatus TEST_Cip_Template1::Create()
{
	CipStatus stat;

	stat.status = kCipGeneralStatusCodeSuccess;
	return stat;
}

CipStatus TEST_Cip_Template2::Create()
{
	CipStatus stat;

	stat.status = kCipGeneralStatusCodeSuccess;
	return stat;
}

int main()
{
	int index1 = -1, index2 = -1;
    //Create two test classes mimicking CIP objects
    index1 = TEST_Cip_Template1::Init().extended_status;
    index2 = TEST_Cip_Template2::Init().extended_status;

	TEST_Cip_Template1 * temp1 = (TEST_Cip_Template1 *)TEST_Cip_Template1::GetInstance(index1);
	TEST_Cip_Template2 * temp2 = (TEST_Cip_Template2 *)TEST_Cip_Template2::GetInstance(index2);


	//Test values
	/*if (0 != *(CipUdint*)(temp1->GetCipAttribute(6)->getData()))//serial number
		return -1;

	if (1 != *(CipUint*)(temp2->GetCipAttribute(2)->getData()))//revision as uint
		return -1;*/

    //Create instances of the two CIP objects
	/*
    index1 = TEST_Cip_Template1::Create().extended_status;
    TEST_Cip_Template1 * temp1 = (TEST_Cip_Template1 *)TEST_Cip_Template1::GetInstance (index);

    index2 = TEST_Cip_Template2::Create().extended_status;
    TEST_Cip_Template2 * temp2 = (TEST_Cip_Template2 *)TEST_Cip_Template2::GetInstance (index);

	

    //Destroy two test classes mimicking CIP objects
    TEST_Cip_Template1::Shut();
    TEST_Cip_Template2::Shut();*/

    return 0;
}