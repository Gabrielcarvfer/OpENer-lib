//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 03/06/2017.
//

#ifndef OPENERMAIN_CIP_OBJECT_H
#define OPENERMAIN_CIP_OBJECT_H

#include <ciptypes.hpp>
#include "template/CIP_Object_template.hpp"

#include "CIP_0001_Identity/CIP_Identity.hpp"
//#include "CIP_0002_MessageRouter/CIP_MessageRouter.hpp"
//#include "CIP_0003_DeviceNET/CIP_DeviceNetLink.hpp"
#include "CIP_0004_Assembly/CIP_Assembly.hpp"
#include "CIP_0005_Connection/CIP_Connection.hpp"
#include "CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
//#include "CIP_000A_AnalogInput/CIP_Analog_Input_Point.hpp"
#include "CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"
#include "CIP_00F6_EthernetLink/CIP_EthernetIP_Link.hpp"

class CIP_Object_glue;



class CIP_Object_glue: public CIP_Object_template<CIP_Object_glue>
{
    public:
    CipStatus InstanceServices(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);

    const CIP_Object_glue * GetInstance(CipUdint instance_number);

        void * retrieveAttribute(CipUsint attributeNumber);
        CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);

};



typedef union
{

#ifdef CIP_CLASSES_IDENTITY_H
    //Cip Identity pointer
    CIP_Identity identity;
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
    //Cip MessageRouter pointer
    //CIP_MessageRouter *messageRouter;
#endif

#ifdef CIP_CLASSES_ASSEMBLY_H
    //Cip Assembly pointer
    CIP_Assembly assembly;
#endif

#ifdef CIP_CLASSES_CONNECTION_H
    //Cip Connection pointer
    CIP_Connection connection;
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
    //Cip ConnectionManager pointer
    CIP_ConnectionManager connectionManager;
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
    //Cip AnalogInputPoint pointer
    CIP_AnalogInputPoint analogInputPoint;
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
    //Cip TCPIPInterface pointer
    CIP_TCPIP_Interface tcpipInterface;
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
    //Cip EthernetLink pointer
    CIP_EthernetIP_Link ethernetIPLink;
#endif

    CIP_Object_glue glue;
} CIP_Object_generic;

#endif //OPENERMAIN_CIP_OBJECT_H
