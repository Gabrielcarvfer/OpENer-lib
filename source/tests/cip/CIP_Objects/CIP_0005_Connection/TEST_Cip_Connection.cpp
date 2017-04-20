
#include "TEST_Cip_Connection.hpp"

int main()
{
	CIP_Connection::Init();

	int instance_number;

	instance_number = CIP_Connection::Create(nullptr, nullptr).extended_status;

	CIP_Connection* instance = nullptr;

	instance = (CIP_Connection*)CIP_Connection::GetInstance(instance_number);

	CIP_Connection::Shut();

	return 0;
}