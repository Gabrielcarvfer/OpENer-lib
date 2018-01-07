//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 02/06/2017.
//

#include <iostream>
#include <ciptypes.hpp>
#include <cip/CIP_Objects/CIP_0002_MessageRouter/CIP_MessageRouter.hpp>

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
    CIP_MessageRouter::RegisterCIPClass((void*)identity_class_ptr,identity_class_ptr->classId);

    //Retrieve registered class instance from msg_router
    CIP_Object_generic * registered_class_ptr = CIP_MessageRouter::GetRegisteredObject(1);

    //Try to access direct values from registered class

    std::cout << "prodcode " << *(char*)registered_class_ptr->glue.retrieveAttribute(1)<< " " << std::endl;

    //Try to retrieve attributes from registered class
    char* attr = (char*)registered_class_ptr->glue.retrieveAttribute(1);

    CIP_Attribute attr1 = registered_class_ptr->glue.GetCipAttribute(1);

    //Try to call services from registered class
    CipMessageRouterRequest_t  req{};
    CipMessageRouterResponse_t resp{};
    CipStatus stat{};

    stat = registered_class_ptr->glue.retrieveService(1,&req,&resp);

    //Exit of registered service call failed
    if (stat.status != kCipGeneralStatusCodeServiceNotSupported)
        exit(-1);

    //Try to shutdown registered class

    //todo:Try to access direct values from message router

    //todo:Try to retrieve attributes from message router

    //todo:Try to call services from message router
    CipEpath path;
    CipNotification notif;
    CIP_MessageRouter::notify_application(path, 1, &notif);

    CIP_Identity::Shut();
    CIP_MessageRouter::Shut();

    return 0;
}