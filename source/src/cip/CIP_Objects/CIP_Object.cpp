//
// Created by gabriel on 9/8/17.
//

#include <ciptypes.hpp>
#include "CIP_Object.hpp"


#define SWITCH_OBJECTS_XY(X,Y) \
case kCipIdentityClassCode: \
return Y((CIP_Identity*) this)->X; \
case kCipAssemblyClassCode: \
return Y((CIP_Assembly *) this)->X; \
case kCipConnectionClassCode: \
return Y((CIP_Connection *) this)->X; \
case kCipConnectionManagerClassCode: \
return Y((CIP_ConnectionManager *) this)->X; \
case kCipTcpIpInterfaceClassCode: \
return Y((CIP_TCPIP_Interface *) this)->X; \
case kCipEthernetLinkClassCode: \
return Y((CIP_EthernetIP_Link *) this)->X; \
/*case kCipAnalogInputPointClassCode: \
return Y((CIP_AnalogInputPoint *) this)->X; \*/

#define SWITCH_OBJECTS_X(X) SWITCH_OBJECTS_XY(X, )

const CIP_Object_glue * CIP_Object_glue::GetInstance(CipUdint this_number)
{
    
    switch (this->classId)
    {
        SWITCH_OBJECTS_XY(GetInstance(this_number),(const CIP_Object_glue*))
        default:
            return nullptr;
    }
}

void* CIP_Object_glue::retrieveAttribute( CipUsint attributeNumber)
{
    switch (this->classId)
    {
        SWITCH_OBJECTS_X(GetCipAttribute(attributeNumber).value_ptr.raw_ptr)
        default:
            return nullptr;
    }
}

CIP_Attribute CIP_Object_glue::GetCipAttribute(CipUsint attribute_number)
{
    switch (this->classId)
    {
        SWITCH_OBJECTS_X(GetCipAttribute(attribute_number))
        default:
            CIP_Attribute attr{kCipAny, nullptr};
            return attr;
    }
}

CipStatus CIP_Object_glue::retrieveService(CipUsint serviceNumber,
                                           CipMessageRouterRequest_t *req,
                                           CipMessageRouterResponse_t *resp)
{
    switch (this->classId)
    {
        SWITCH_OBJECTS_X(InstanceServices(serviceNumber,req,resp))
        default:
            CipStatus stat;
            stat.status = kCipGeneralStatusCodeServiceNotSupported;
            return stat;
    }
};



