//
// Created by gabriel on 02/06/2017.
//

#include "TEST_CIP_MessageRouter.h"
#include <iostream>
#include <cip/ciptypes.hpp>
#include <cip/CIP_Objects/CIP_ClassStack.hpp>

int main()
{
    //Init Message router structure in memory and register its attributes+services
    CIP_MessageRouter::Init();

    //Init CIP_Identity  structure in memory and register its attributes+services
    CIP_Identity::Init();

    //Get CIP_Identity class instance
    const CIP_Identity* identity_class_ptr = nullptr;
    identity_class_ptr = CIP_Identity::GetInstance(0);

    //Register CIP_Identity on message router
    CIP_MessageRouter::RegisterCIPClass((void*)identity_class_ptr,identity_class_ptr->class_id);

    //Retrieve registered class instance from msg_router
    CIP_Object * registered_class_ptr = CIP_MessageRouter::GetRegisteredObject(1);

    //Try to access direct values
    std::cout << "prodcode " << registered_class_ptr->identity->revision.major_revision << " " << std::endl;

    //Try to retrieve attributes

    //Try to call services

    //Try to shutdown
    registered_class_ptr->identity->Shut();

    CIP_MessageRouter::Shut();

    return 0;
}