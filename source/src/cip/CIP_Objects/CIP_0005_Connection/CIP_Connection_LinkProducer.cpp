//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#include "CIP_Connection_LinkProducer.hpp"

CIP_Connection_LinkProducer::CIP_Connection_LinkProducer(CipUsint state, CipUint Connection_id)
{
    this->State = state;
    this->Connection_id = Connection_id;
}

CIP_Connection_LinkProducer::~CIP_Connection_LinkProducer ()
{

}

CipStatus CIP_Connection_LinkProducer::Send()
{

}

