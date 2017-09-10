//
// Created by gabriel on 9/8/17.
//

#include <ciptypes.hpp>
#include "CIP_Object.hpp"


#include "CIP_0001_Identity/CIP_Identity.hpp"
//#include "CIP_0002_MessageRouter/CIP_MessageRouter.hpp"
//#include "CIP_0003_DeviceNET/CIP_DeviceNetLink.hpp"
#include "CIP_0004_Assembly/CIP_Assembly.hpp"
#include "CIP_0005_Connection/CIP_Connection.hpp"
#include "CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "CIP_000A_AnalogInput/CIP_Analog_Input_Point.hpp"
#include "CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"
#include "CIP_00F6_EthernetLink/CIP_EthernetIP_Link.hpp"

const CIP_Object_glue * CIP_Object_glue::GetInstance(CipUdint this_number)
{
    
    switch (this->classId)
    {
#ifdef CIP_CLASSES_IDENTITY_H
        case kCipIdentityClassCode:
            //Cip Identity pointer
            return (const CIP_Object_glue*)((CIP_Identity*) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
        case kCipMessageRouterClassCode:
            //Cip MessageRouter pointer
            //CIP_MessageRouter * messageRouter;
            break;
#endif

#ifdef CIP_CLASSES_ASSEMBLY_H
        case kCipAssemblyClassCode:
            //Cip Assembly pointer
            return (const CIP_Object_glue*)((CIP_Assembly *) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_CONNECTION_H
        case kCipConnectionClassCode:
            //Cip Connection pointer
            return (const CIP_Object_glue*)((CIP_Connection *) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
        case kCipConnectionManagerClassCode:
            //Cip ConnectionManager pointer
            return (const CIP_Object_glue*)((CIP_ConnectionManager *) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
        case kCipAnalogInputPointClassCode:
            //Cip AnalogInputPoint pointer
            return (const CIP_Object_glue*)((CIP_AnalogInputPoint *) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
        case kCipTcpIpInterfaceClassCode:
            //Cip TCPIPInterface pointer
            return (const CIP_Object_glue*)((CIP_TCPIP_Interface *) this)->GetInstance(this_number);
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
        case kCipEthernetLinkClassCode:
            //Cip EthernetLink pointer
            return (const CIP_Object_glue*)((CIP_EthernetIP_Link *) this)->GetInstance(this_number);
#endif
        default:
            return nullptr;
    }
}

void* CIP_Object_glue::retrieveAttribute( CipUsint attributeNumber)
{
    switch (this->classId) {
#ifdef CIP_CLASSES_IDENTITY_H
        case kCipIdentityClassCode:
            //Cip Identity pointer
            return ((CIP_Identity*) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
        case kCipMessageRouterClassCode:
            //Cip MessageRouter pointer
            //CIP_MessageRouter * messageRouter;
            break;
#endif

#ifdef CIP_CLASSES_ASSEMBLY_H
        case kCipAssemblyClassCode:
            //Cip Assembly pointer
            return ((CIP_Assembly *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_CONNECTION_H
        case kCipConnectionClassCode:
            //Cip Connection pointer
            return ((CIP_Connection *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
        case kCipConnectionManagerClassCode:
            //Cip ConnectionManager pointer
            return ((CIP_ConnectionManager *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
        case kCipAnalogInputPointClassCode:
            //Cip AnalogInputPoint pointer
            return ((CIP_AnalogInputPoint *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
        case kCipTcpIpInterfaceClassCode:
            //Cip TCPIPInterface pointer
            return ((CIP_TCPIP_Interface *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
        case kCipEthernetLinkClassCode:
            //Cip EthernetLink pointer
            return ((CIP_EthernetIP_Link *) this)->GetCipAttribute(attributeNumber).value_ptr.raw_ptr;
#endif
        default:
            return nullptr;
    }
}

CipStatus CIP_Object_glue::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req,
                                           CipMessageRouterResponse_t *resp){
    switch (this->classId)
    {
#ifdef CIP_CLASSES_IDENTITY_H
        case kCipIdentityClassCode:
            //Cip Identity pointer
            return ((CIP_Identity*) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_MESSAGEROUTER_H
        case kCipMessageRouterClassCode:
            //Cip MessageRouter pointer
            //CIP_MessageRouter * messageRouter;
            break;
#endif

#ifdef CIP_CLASSES_ASSEMBLY_H
        case kCipAssemblyClassCode:
            //Cip Assembly pointer
            return ((CIP_Assembly *) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_CONNECTION_H
        case kCipConnectionClassCode:
            //Cip Connection pointer
            return ((CIP_Connection *) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_CONNECTIONMANAGER_H
        case kCipConnectionManagerClassCode:
            //Cip ConnectionManager pointer
            return ((CIP_ConnectionManager *) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_ANALOGINPUTPOINT_H
        case kCipAnalogInputPointClassCode:
            //Cip AnalogInputPoint pointer
            return ((CIP_AnalogInputPoint *) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_TCPIPINTERFACE_H
        case kCipTcpIpInterfaceClassCode:
            //Cip TCPIPInterface pointer
            return ((CIP_TCPIP_Interface *) this)->InstanceServices(serviceNumber, req, resp);
#endif

#ifdef CIP_CLASSES_ETHERNETLINK_H
        case kCipEthernetLinkClassCode:
            //Cip EthernetLink pointer
            return ((CIP_EthernetIP_Link *) this)->InstanceServices(serviceNumber, req, resp);
#endif
        default:
            CipStatus stat;
            stat.status = kCipGeneralStatusCodeServiceNotSupported;
            return stat;
    }
};
