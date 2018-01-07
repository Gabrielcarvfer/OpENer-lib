//
// Created by gabriel on 10/12/2017.
//

#ifndef OPENER_CIP_OBJECT_BASE_H
#define OPENER_CIP_OBJECT_BASE_H

#include <ciptypes.hpp>

class CIP_Object_base {
    public:
        CipUint classId;
        static int EncodeData (CipUsint cip_type, void *data, std::vector<CipUsint> *message);
        static int DecodeData (CipUsint cip_type, void *data, CipUsint *message);




};


#endif //OPENER_CIP_OBJECT_BASE_H
