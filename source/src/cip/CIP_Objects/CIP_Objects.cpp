//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 26/01/2017.
//

#include "CIP_Objects.hpp"
#include "CIP_0001_Identity/CIP_Identity.hpp"
#include "CIP_0002_MessageRouter/CIP_MessageRouter.hpp"
//#include "CIP_0003_DeviceNET/CIP_DeviceNetLink.hpp"
#include "CIP_0004_Assembly/CIP_Assembly.hpp"
#include "CIP_0005_Connection/CIP_Connection.hpp"
#include "CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "CIP_000A_AnalogInput/CIP_Analog_Input_Point.hpp"
#include "CIP_00F5_TCPIP_Interface/CIP_EthIP_Interface.hpp"
#include "CIP_00F6_EthernetLink/CIP_EthIP_Link.hpp"

int CIP_Objects::InitObjects()
{
    //init CIP Identiry object and return object ID in case of failure
    if (CIP_Identity::Init().status != kCipStatusOk)
        return CIP_Identity::class_id;

    if (CIP_MessageRouter::Init().status != kCipStatusOk)
        return CIP_MessageRouter::class_id;

    //if (CIP_DeviceNET_Link::Init().status != kCipStatusOk)
    //    return CIP_DeviceNET_Link::class_id;

    if (CIP_Assembly::Init().status != kCipStatusOk)
        return CIP_Assembly ::class_id;

    if (CIP_Connection::Init().status != kCipStatusOk)
        return CIP_Connection::class_id;

    if (CIP_ConnectionManager::Init().status != kCipStatusOk)
        return CIP_ConnectionManager::class_id;

    if (CIP_AnalogInputPoint::Init().status != kCipStatusOk)
        return CIP_AnalogInputPoint::class_id;

    if (CIP_EthIP_Interface::Init().status != kCipStatusOk)
        return CIP_EthIP_Interface::class_id;

    if (CIP_EthIP_Link::Init().status != kCipStatusOk)
        return CIP_EthIP_Link::class_id;

    return 0; //OK
}

int CIP_Objects::ShutObjects()
{
    //shut CIP Identiry object and return object ID in case of failure
    if (CIP_Identity::Shut().status != kCipStatusOk)
        return CIP_Identity::class_id;

    if (CIP_MessageRouter::Shut().status != kCipStatusOk)
        return CIP_MessageRouter::class_id;

    //if (CIP_DeviceNET_Link::Shut().status != kCipStatusOk)
    //    return CIP_DeviceNET_Link::class_id;

    if (CIP_Assembly::Shut().status != kCipStatusOk)
        return CIP_Assembly ::class_id;

    if (CIP_Connection::Shut().status != kCipStatusOk)
        return CIP_Connection::class_id;

    if (CIP_ConnectionManager::Shut().status != kCipStatusOk)
        return CIP_ConnectionManager::class_id;

    if (CIP_AnalogInputPoint::Shut().status != kCipStatusOk)
        return CIP_AnalogInputPoint::class_id;

    if (CIP_EthIP_Interface::Shut().status != kCipStatusOk)
        return CIP_EthIP_Interface::class_id;

    if (CIP_EthIP_Link::Shut().status != kCipStatusOk)
        return CIP_EthIP_Link::class_id;

    return 0; //OK
}