//
// Created by gabriel on 18/11/2016.
//

#ifndef OPENER_CIP_TEMPLATE_H
#define OPENER_CIP_TEMPLATE_H

#include "../ciptypes.h"
#include <map>

template <typename T>
class CIP_Template
{
    public:
        static T * GetInstance(CipUdint instance_number);
        static T * GetClass();
        static CipUdint GetNumberOfInstances();
        static CipUdint GetInstanceNumber(T * instance);
        static bool AddClassInstance(T* instance, CipUdint position);
        static bool RemoveClassInstance(T* instance);
        static bool RemoveClassInstance(CipUdint position);
    protected:
        static std::map<CipUdint, T*> object_Set;
};


#endif //OPENER_CIP_TEMPLATES_H
