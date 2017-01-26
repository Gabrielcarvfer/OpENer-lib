//
// Created by gabriel on 26/01/2017.
//

#include "CIP_Objects.hpp"
#include "CIP_0001_Identity/CIP_Identity.hpp"
#include "CIP_0002_MessageRouter/CIP_MessageRouter.hpp"
#include "CIP_0003_DeviceNET/CIP_DeviceNetLink.hpp"
#include "CIP_0004_Assembly/CIP_Assembly.hpp"
#include "CIP_0005_Connection/CIP_Connection.hpp"
#include "CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "CIP_000A_AnalogInput/CIP_Analog_Input_Point.hpp"
#include "CIP_00F5_TCPIP_Interface/CIP_EthIP_Interface.hpp"
#include "CIP_00F6_EthernetLink/CIP_EthIP_Link.hpp"

int CIP_Objects::InitObjects()
{
    CIP_Identity::Init();
    CIP_MessageRouter::Init();
    CIP_DeviceNET_Link::Init();
    CIP_Assembly::Init();
    CIP_Connection::Init();
    CIP_ConnectionManager::Init();
    CIP_AnalogInputPoint::Init();
    CIP_EthIP_Interface::Init();
    CIP_EthIP_Link::Init();
    return -1; //OK
}

int CIP_Objects::ShutObjects()
{
    CIP_Identity::Shut();
    CIP_MessageRouter::Shut();
    CIP_DeviceNET_Link::Shut();
    CIP_Assembly::Shut();
    CIP_Connection::Shut();
    CIP_ConnectionManager::Shut();
    CIP_AnalogInputPoint::Shut();
    CIP_EthIP_Interface::Shut();
    CIP_EthIP_Link::Shut();
    return -1; //OK
}