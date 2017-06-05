//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 20/04/2017.
//

#include "TEST_Cip_Connection.hpp"
#include <iostream>

int main()
{
	CIP_Connection::Init();

	int instance_number;

    CipStatus stat;

	stat = CIP_Connection::Create(nullptr, nullptr);

    if (stat.status != kCipStatusOk)
        exit(-1);

    instance_number = stat.extended_status;

	CIP_Connection* instance = (CIP_Connection*)CIP_Connection::GetInstance(instance_number);

    std::cout << instance->id << std::endl;

	CIP_Connection::Shut();

	return 0;
}