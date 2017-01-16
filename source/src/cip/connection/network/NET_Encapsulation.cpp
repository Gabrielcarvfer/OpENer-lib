

#include "NET_Encapsulation.hpp"
#include "ethIP/NET_EthIP_Encap.hpp"

CipStatus NET_Encapsulation::Initialize ()
{
    //Initialize supported protocol encapsulations
    NET_EthIP_Encap::EncapsulationInit ();

	return kCipStatusOk;
}

CipStatus NET_Encapsulation::Shutdown()
{
    //Shutdown supported protocol encapsulations
    NET_EthIP_Encap::EncapsulationShutDown ();
	return kCipStatusOk;
}