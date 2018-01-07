//
// Created by Gabriel Ferreira (@gabrielcarvfer) on 2/19/2017.
//

#ifndef OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP
#define OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP

#include <ciptypes.hpp>

class CIP_Connection_LinkConsumer
{
public:
    CIP_Connection_LinkConsumer(CipUsint state = non_existant, CipUint Connection_id = 0);
    ~CIP_Connection_LinkConsumer ();

    //Instance definitions
    typedef enum {
        non_existant = 0, //link producer not instantiated
        running      = 1//instantiated and waiting for receive request
    }link_producer_state_e;

    //Instance attributes
    CipUsint State;
    CipUint  Connection_id;//todo: recheck size/reference

    //Instance services
    CipStatus Receive();

    private:
};


#endif //OPENERMAIN_CIP_CONNECTION_LINKCONSUMER_HPP
