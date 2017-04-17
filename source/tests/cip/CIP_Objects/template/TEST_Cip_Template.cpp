//
// Created by Gabriel Ferreira (@gabrielcarvfer)  on 17/04/2017.
//

#include "TEST_Cip_Template.hpp"


CipUint TEST_Cip_Template1::vendor_id_;
CipUint TEST_Cip_Template1::device_type_;
CipUint TEST_Cip_Template1::product_code_;
CipRevision TEST_Cip_Template1::revision_;
CipUint TEST_Cip_Template1::status_;
CipUdint TEST_Cip_Template1::serial_number_;
CipShortString TEST_Cip_Template1::product_name_;

CipUint TEST_Cip_Template2::product_code_;
CipUint TEST_Cip_Template2::revision_;


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

		instance->InsertAttribute(1, kCipUint, &vendor_id_, kGetableSingleAndAll);
		instance->InsertAttribute(2, kCipUint, &device_type_, kGetableSingleAndAll);
		instance->InsertAttribute(3, kCipUint, &product_code_, kGetableSingleAndAll);
		instance->InsertAttribute(4, kCipUsintUsint, &revision_, kGetableSingleAndAll);
		instance->InsertAttribute(5, kCipWord, &status_, kGetableSingleAndAll);
		instance->InsertAttribute(6, kCipUdint, &serial_number_, kGetableSingleAndAll);
		instance->InsertAttribute(7, kCipShortString, &product_name_, kGetableSingleAndAll);

		stat.status = kCipGeneralStatusCodeSuccess;
		stat.extended_status = 0;
	}
	else
	{
		stat.status = kCipGeneralStatusCodeObjectAlreadyExists;
	}
	return stat;
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

		instance->InsertAttribute(1, kCipUint, &product_code_, kGetableSingleAndAll);
		instance->InsertAttribute(2, kCipUsintUsint, &revision_, kGetableSingleAndAll);

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
	if (0 != *(CipUdint*)(temp1->GetCipAttribute(6)->getData()))//serial number
		return -1;

	if (1 != *(CipUint*)(temp2->GetCipAttribute(2)->getData()))//revision as uint
		return -1;

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