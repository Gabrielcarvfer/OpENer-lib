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
#include "CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"
#include "CIP_00F6_EthernetLink/CIP_EthernetIP_Link.hpp"

int CIP_Objects::InitObjects()
{
#ifdef OPENER_CIPIDENTITY_H_
    //init CIP Identiry object and return object ID in case of failure
    if (CIP_Identity::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_Identity::class_id;
#endif

#ifdef OPENER_CIPMESSAGEROUTER_H_
    if (CIP_MessageRouter::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_MessageRouter::class_id;
#endif

    //if (CIP_DeviceNET_Link::Init().status != kCipGeneralStatusCodeSuccess)
    //    return CIP_DeviceNET_Link::class_id;

#ifdef OPENER_CIPASSEMBLY_H_
    if (CIP_Assembly::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_Assembly ::class_id;
#endif

#ifdef OPENERMAIN_CIP_CONNECTION_H
    if (CIP_Connection::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_Connection::class_id;
#endif

#ifdef OPENER_CIP_CONNECTION_H_
    if (CIP_ConnectionManager::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_ConnectionManager::class_id;
#endif

#ifdef OPENER_CLASSES_ANALOG_INPUT_POINT_H_
    if (CIP_AnalogInputPoint::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_AnalogInputPoint::class_id;
#endif

#ifdef OPENER_CIPTCPIPINTERFACE_H_
    if (CIP_TCPIP_Interface::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_TCPIP_Interface::class_id;
#endif

#ifdef OPENER_CIPETHERNETLINK_H_
    if (CIP_EthernetIP_Link::Init().status != kCipGeneralStatusCodeSuccess)
        return CIP_EthernetIP_Link::class_id;
#endif

    return 0; //OK
}

int CIP_Objects::ShutObjects()
{
    //shut CIP Identiry object and return object ID in case of failure
    if (CIP_Identity::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_Identity::class_id;

    if (CIP_MessageRouter::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_MessageRouter::class_id;

    //if (CIP_DeviceNET_Link::Shut().status != kCipGeneralStatusCodeSuccess)
    //    return CIP_DeviceNET_Link::class_id;

    if (CIP_Assembly::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_Assembly ::class_id;

    if (CIP_Connection::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_Connection::class_id;

    if (CIP_ConnectionManager::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_ConnectionManager::class_id;

    if (CIP_AnalogInputPoint::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_AnalogInputPoint::class_id;

    if (CIP_TCPIP_Interface::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_TCPIP_Interface::class_id;

    if (CIP_EthernetIP_Link::Shut().status != kCipGeneralStatusCodeSuccess)
        return CIP_EthernetIP_Link::class_id;

    return 0; //OK
}