

#include "NET_Encapsulation.hpp"
#include "ethernetip_net/NET_EthIP_Encap.hpp"

CipStatus NET_Encapsulation::Initialize ()
{
    //Initialize supported protocol encapsulations
    NET_EthIP_Encap::EncapsulationInit ();


}

CipStatus NET_Encapsulation::Shutdown()
{
    //Shutdown supported protocol encapsulations
    NET_EthIP_Encap::EncapsulationShutDown ();
}