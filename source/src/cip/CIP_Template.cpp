//
// Created by gabriel on 18/11/2016.
//

#include "CIP_Template.h"

template <typename T>
T * CIP_Template::GetCipClassInstance(CipUdint class_id, CipUdint instance_number)
{
    if (object_set[class_id].size() >= instance_number)
        return CIP_object_set[class_id][instance_number];
    else
        return NULL;
}

template <typename T>
T * CIP_Template::GetCipClass(CipUdint class_id)
{
    if (object_set[class_id].size() > 0)
        return object_set[class_id][0];
    else
        return NULL;
}

template <typename T>
CipUdint CIP_Template::GetCipClassNumberInstances(CipUdint class_id)
{
    return object_set[class_id].size();
}

template <typename T>
CipUdint CIP_Template::GetCipInstanceNumber(T * instance)
{
    return std::distance(object_set[instance->class_id].begin(), object_set[instance->class_id].find(instance) );
}

template <typename T>
bool CIP_Template::AddCipClassInstance(T* instance, CipUdint position)
{
    object_set[instance->class_id].emplace(position,instance);
    auto it = object_set[instance->class_id].find(position);
    return (it != Cobject_set[instance->class_id].end());
}