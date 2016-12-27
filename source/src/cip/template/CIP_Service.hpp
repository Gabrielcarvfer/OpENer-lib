//
// Created by gabriel on 11/11/2016.
//

#ifndef OPENER_CIP_SERVICES_H
#define OPENER_CIP_SERVICES_H

#include "../ciptypes.hpp"
#include <string>

class CIP_Service
{
    public:
        //Constructor and deconstructor
        CIP_Service(CipUsint service_number, CipServiceFunction* service_function, std::string name);
        ~CIP_Service ();

        //Functions avaiable
        CipUsint getNumber();
        //CipServiceFunction getService();
        std::string getName();

    private:
        CipUsint service_number; /**< service number*/
        CipServiceFunction* service_function;//CipServiceFunction service_function; /**< pointer to a function call*/
        std::string name; /**< name of the service */
};

#endif //OPENER_CIP_SERVICES_H
