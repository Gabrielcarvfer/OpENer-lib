//
// Created by gabriel on 11/11/2016.
//

#include "CIP_Service.hpp"

CIP_Service::CIP_Service(CipUsint service_number, CipServiceFunction* service_function, std::string name)
{
    // fill in function number
    this->service_number = service_number;

    // fill in function address
    this->service_function = service_function;

    // fill in function name
    this->name = name;
}

CIP_Service::~CIP_Service ()
{

}

CipUsint CIP_Service::getNumber()
{
    return this->service_number;
}

CipServiceFunction CIP_Service::getService()
{
    //return this->service_function;
}

std::string CIP_Service::getName()
{
    return this->name;
}