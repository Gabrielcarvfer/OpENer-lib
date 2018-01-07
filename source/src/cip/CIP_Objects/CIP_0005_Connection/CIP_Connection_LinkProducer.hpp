//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#ifndef OPENERMAIN_CIP_CONNECTION_LINKPRODUCER_HPP
#define OPENERMAIN_CIP_CONNECTION_LINKPRODUCER_HPP

#include <ciptypes.hpp>

class CIP_Connection_LinkProducer
{
public:
    CIP_Connection_LinkProducer(CipUsint state = non_existant, CipUint Connection_id = 0);
    ~CIP_Connection_LinkProducer ();

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
