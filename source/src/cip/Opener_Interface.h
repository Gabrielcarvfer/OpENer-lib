//
// Created by gabriel on 11/11/2016.
//

#ifndef OPENER_OPENER_INTERFACE_H
#define OPENER_OPENER_INTERFACE_H


class Opener_Interface
{
    public:
        //Initialize or shutdown Opener CIP stack
        static Opener_Initialize();
        static Opener_Shutdown();

        //Services provided
        //Explicit connections return an object so that you can send data you want, to whom you want and desired network
        static Opener_CreateExplicitConnection();
        static Opener_RemoveExplicitConnection();

        //Implicit connections receive a pointer and block size, plus whom you want to send and desired network
        static Opener_CreateIOConnection();
        static Opener_RemoveIOConnection();
    private:
        //Stuff necessary to support services based on
        //class_stack
        //connection_stack
        //network_stack
};


#endif //OPENER_OPENER_INTERFACE_H
