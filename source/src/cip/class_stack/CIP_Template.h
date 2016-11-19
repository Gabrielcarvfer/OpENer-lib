//
// Created by gabriel on 18/11/2016.
//

#ifndef OPENER_CIP_TEMPLATE_H
#define OPENER_CIP_TEMPLATE_H

template <typename T>
class CIP_Template
{
    public:
        static T * GetCipClassInstance(CipUdint class_id, CipUdint instance_number);
        static T * GetCipClass(CipUdint class_id);
        static CipUdint GetCipClassNumberInstances(CipUdint class_id);
        static CipUdint GetCipInstanceNumber(T * instance);
        static bool AddCipClassInstance(T* instance, CipUdint position);
    private:
        static std::map<CipUdint, T*> object_Set;
};


#endif //OPENER_CIP_TEMPLATES_H
