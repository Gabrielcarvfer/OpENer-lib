//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#include "CIP_Connection_LinkConsumer.hpp"

CIP_Connection_LinkConsumer::CIP_Connection_LinkConsumer()
{

}
CIP_Connection_LinkConsumer::~CIP_Connection_LinkConsumer ()
{

}

//Class services

CipStatus CIP_Connection_LinkConsumer::Init()
{
    if (number_of_instances == 0)
    {
        CIP_Connection_LinkConsumer *instance = new CIP_Connection_LinkConsumer();

        object_Set.emplace(object_Set.size(), instance);

        //Class services
    }
    return kCipStatusOk;
}

CipStatus CIP_Connection_LinkConsumer::Create()
{

    CIP_Connection_LinkConsumer *instance = new CIP_Connection_LinkConsumer();
    //Chapter 3-4.4 vol 1
    instance->InsertAttribute( 1, kCipUsint, &instance->State        , kGetableSingleAndAll);
    instance->InsertAttribute( 2, kCipUint , &instance->Connection_id, kGetableSingleAndAll);

    object_Set.emplace(object_Set.size(), instance);

    CipStatus stat;
    stat.status = kCipStatusOk;
    stat.extended_status = (CipUsint) instance->id;
    return stat;
}

CipStatus CIP_Connection_LinkConsumer::Delete(CIP_Connection_LinkConsumer * link_instance)
{

}

//Instance services
CipStatus CIP_Connection_LinkConsumer::Receive()
{

}

CipStatus CIP_Connection_LinkConsumer::InstanceServices(int service, CipMessageRouterRequest_t * msg_router_request,CipMessageRouterResponse_t* msg_router_response)
{

}