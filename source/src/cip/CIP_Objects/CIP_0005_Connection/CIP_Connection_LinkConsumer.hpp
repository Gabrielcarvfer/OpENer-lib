//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#ifndef OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP
#define OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP


#include <cip/CIP_Objects/template/CIP_Object.hpp>

class CIP_Connection_LinkConsumer : public CIP_Object<CIP_Connection_LinkConsumer>
{
public:
    CIP_Connection_LinkConsumer();
    ~CIP_Connection_LinkConsumer ();

    //Class services
    static CipStatus Init();
    static CipStatus Create();
    static CipStatus Delete(CIP_Connection_LinkConsumer * link_instance);


    //Instance definitions
    typedef enum {
        non_existant, //link producer not instantiated
        running //instantiated and waiting for receive request
    }link_producer_state_e;

    //Instance attributes
    CipUsint State;
    CipUint Connection_id;//todo: recheck size/reference

    //Instance services
    CipStatus Receive();

    private:
        CipStatus InstanceServices(int service, CipMessageRouterRequest_t * msg_router_request,CipMessageRouterResponse_t* msg_router_response);

};


#endif //OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP
