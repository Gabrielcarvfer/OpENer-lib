//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#include "CIP_Connection_LinkConsumer.hpp"

CIP_Connection_LinkConsumer::CIP_Connection_LinkConsumer(CipUsint state, CipUint Connection_id)
{
    this->State = state;
    this->Connection_id = Connection_id;
}
CIP_Connection_LinkConsumer::~CIP_Connection_LinkConsumer ()
{

}

CipStatus CIP_Connection_LinkConsumer::Receive()
{

}
