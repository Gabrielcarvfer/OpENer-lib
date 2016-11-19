//
// Created by gabriel on 11/11/2016.
//

#ifndef OPENER_OPENER_INTERFACE_H
#define OPENER_OPENER_INTERFACE_H

#include <map>

#include "typedefs.h"
#include "Opener_ExplicitConnection.h"
#include "Opener_IOConnection.h"

class Opener_Interface
{
    public:
        //Initialize or shutdown Opener CIP stack
        static bool Opener_Initialize(CipUdint serialNumber);
        static bool Opener_Shutdown();

        //Services provided
        //Explicit connections return an object so that you can send data you want, to whom you want and desired network
        static CipUdint Opener_CreateExplicitConnection();
        static void Opener_RemoveExplicitConnection(CipUdint handle);

        //Implicit connections receive a pointer and block size, plus whom you want to send and desired network
        static CipUdint Opener_CreateIOConnection();
        static void Opener_RemoveIOConnection(CipUdint handle);

        //Get pointers to use
        static Opener_IOConnection * GetOpenerIOConnection(CipUdint handle);
        static Opener_ExplicitConnection * GetOpenerExplicitConnection(CipUdint handle);
    private:
        static std::map<CipUdint, Opener_IOConnection*> IO_Connection_set;
        static std::map<CipUdint, Opener_ExplicitConnection*> Explicit_Connection_set;
};


#endif //OPENER_OPENER_INTERFACE_H
