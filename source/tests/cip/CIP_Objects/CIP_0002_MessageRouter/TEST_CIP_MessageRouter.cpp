//
// Created by gabriel on 02/06/2017.
//

#include "TEST_CIP_MessageRouter.h"

int main()
{
    CIP_MessageRouter::Init();

    int instance_number;

    instance_number = CIP_MessageRouter::Create().extended_status;

    CIP_MessageRouter* instance = nullptr;

    //instance = (CIP_MessageRouter*)CIP_MessageRouter::GetInstance(instance_number);

    CIP_MessageRouter::Shut();

    return 0;
}