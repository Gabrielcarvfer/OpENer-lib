//
// Created by Gabriel Ferreira (@gabrielcarvfer)  on 17/04/2017.
//

#include "TEST_Cip_Identity.hpp"

#include <iostream>

int main()
{
	if (CIP_Identity::Init().status == kCipStatusOk)
	{

		CIP_Identity *identity_instance = (CIP_Identity *) CIP_Identity::GetInstance(0);

		identity_instance->product_code = 0;

		std::cout << "prodcode " << identity_instance->product_code << " " << std::endl;
		CIP_Identity::Shut();

		return 0;
	}
	else
		return -1;
}