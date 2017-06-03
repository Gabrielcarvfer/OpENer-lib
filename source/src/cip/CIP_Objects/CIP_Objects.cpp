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

#ifdef CIP_CLASSES_IDENTITY_H
    //init CIP Identity object and return object ID in case of failure
    if (CIP_Identity::Init().status != kCipStatusOk)
        return CIP_Identity::class_id;
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
    if (CIP_MessageRouter::Init().status != kCipStatusOk)
        return CIP_MessageRouter::class_id;
#endif

    //if (CIP_DeviceNET_Link::Init().status != kCipStatusOk)
    //    return CIP_DeviceNET_Link::class_id;

#ifdef CIP_CLASSES_ASSEMBLY_H
    if (CIP_Assembly::Init().status != kCipStatusOk)
        return CIP_Assembly ::class_id;
#endif

#ifdef CIP_CLASSES_CONNECTION_H
    if (CIP_Connection::Init().status != kCipStatusOk)
        return CIP_Connection::class_id;
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
    if (CIP_ConnectionManager::Init().status != kCipStatusOk)
        return CIP_ConnectionManager::class_id;
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
    //if (CIP_AnalogInputPoint::Init().status != kCipStatusOk)
    //    return CIP_AnalogInputPoint::class_id;
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
    if (CIP_TCPIP_Interface::Init().status != kCipStatusOk)
        return CIP_TCPIP_Interface::class_id;
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
    if (CIP_EthernetIP_Link::Init().status != kCipStatusOk)
        return CIP_EthernetIP_Link::class_id;
#endif

    return kCipStatusOk;
}

int CIP_Objects::ShutObjects()
{
#ifdef CIP_CLASSES_IDENTITY_H
    //shutdown CIP Identity object and return object ID in case of failure
    if (CIP_Identity::Shut().status != kCipStatusOk)
        return CIP_Identity::class_id;
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
    if (CIP_MessageRouter::Shut().status != kCipStatusOk)
        return CIP_MessageRouter::class_id;
#endif

    //if (CIP_DeviceNET_Link::Shut().status != kCipStatusOk)
    //    return CIP_DeviceNET_Link::class_id;

#ifdef CIP_CLASSES_ASSEMBLY_H
    if (CIP_Assembly::Shut().status != kCipStatusOk)
        return CIP_Assembly ::class_id;
#endif

#ifdef CIP_CLASSES_CONNECTION_H
    if (CIP_Connection::Shut().status != kCipStatusOk)
        return CIP_Connection::class_id;
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
    if (CIP_ConnectionManager::Shut().status != kCipStatusOk)
        return CIP_ConnectionManager::class_id;
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
    //if (CIP_AnalogInputPoint::Shut().status != kCipStatusOk)
    //    return CIP_AnalogInputPoint::class_id;
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
    if (CIP_TCPIP_Interface::Shut().status != kCipStatusOk)
        return CIP_TCPIP_Interface::class_id;
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
    if (CIP_EthernetIP_Link::Shut().status != kCipStatusOk)
        return CIP_EthernetIP_Link::class_id;
#endif

    return kCipStatusOk;
}