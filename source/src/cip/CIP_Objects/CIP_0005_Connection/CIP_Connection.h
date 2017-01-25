//
// Created by gabriel on 25/01/17.
//

#ifndef OPENERMAIN_CIP_CONNECTION_H
#define OPENERMAIN_CIP_CONNECTION_H

#include "../template/CIP_Object.hpp"

class CIP_Connection : public CIP_Object<CIP_Connection>
{
    public:
    static CipStatus Init();
    CipStatus InstanceServices(int service, CipMessageRouterRequest* msg_router_request, CipMessageRouterResponse* msg_router_response)

};


#endif //OPENERMAIN_CIP_CONNECTION_H
