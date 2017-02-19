//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#include "CIP_Connection_LinkProducer.hpp"

//Class services

CipStatus CIP_Connection_LinkProducer::Init()
{
    if (number_of_instances == 0)
    {
        CIP_Connection_LinkProducer *instance = new CIP_Connection_LinkProducer();

        object_Set.emplace(object_Set.size(), instance);

        //Class services
    }
    return kCipStatusOk;
}

CipStatus CIP_Connection_LinkProducer::Create()
{

    CIP_Connection_LinkProducer *instance = new CIP_Connection_LinkProducer();
    //Chapter 3-4.4 vol 1
    instance->InsertAttribute( 1, kCipUsint, &State        , kGetableSingleAndAll);
    instance->InsertAttribute( 2, kCipUint , &Connection_id, kGetableSingleAndAll);

    object_Set.emplace(object_Set.size(), instance);

    CipStatus stat;
    stat.status = kCipStatusOk;
    stat.extended_status = (CipUsint) instance->id;
    return stat;
}

CipStatus CIP_Connection_LinkProducer::Delete(CIP_Connection_LinkProducer * link_instance)
{

}

//Instance services
CipStatus CIP_Connection_LinkProducer::Send()
{

}