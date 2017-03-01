//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#ifndef OPENERMAIN_CIP_CONNECTION_LINKPRODUCER_HPP
#define OPENERMAIN_CIP_CONNECTION_LINKPRODUCER_HPP


#include <cip/CIP_Objects/template/CIP_Object.hpp>

class CIP_Connection_LinkProducer : public CIP_Object<CIP_Connection_LinkProducer>
{
public:
    //Class services
    static CipStatus Init();
    static CipStatus Create();
    static CipStatus Delete(CIP_Connection_LinkProducer * link_instance);


    //Instance definitions
    typedef enum {
        non_existant, //link producer not instantiated
        running //instantiated and waiting for send request
    }link_producer_state_e;

    //Instance attributes
    CipUsint State;
    CipUint Connection_id;

    //Instance services
    CipStatus Send();
};


#endif //OPENERMAIN_CIP_CONNECTION_LINKPRODUCER_HPP
