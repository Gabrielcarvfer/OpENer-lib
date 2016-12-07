//
// Created by gabriel on 18/11/2016.
//

#include "CIP_Template.h"

template <typename T>
T * CIP_Template::GetInstance(CipUdint instance_number)
{
    if (object_Set.size() >= instance_number)
        return CIP_object_Set[instance_number];
    else
        return NULL;
}

template <typename T>
T * CIP_Template::GetClass()
{
    GetInstance(0);
}

template <typename T>
CipUdint CIP_Template::GetNumberOfInstances()
{
    return object_Set.size();
}

template <typename T>
CipUdint CIP_Template::GetInstanceNumber(T * instance)
{
    return std::distance(object_Set.begin(), object_Set.find(instance) );
}

template <typename T>
bool CIP_Template::AddClassInstance(T* instance, CipUdint position)
{
    object_Set.emplace(position,instance);
    auto it = object_Set.find(position);
    return (it != object_Set.end());
}

template <typename T>
bool CIP_Template::RemoveClassInstance(T* instance)
{
    for (auto it = object_Set.begin(); it != object_Set.end(); it++)
    {
        if (it->second == instance)
        {
            object_Set.erase(it);
            return true;
        }
    }
    return false;
}

template <typename T>
bool CIP_Template::RemoveClassInstance(CipUdint position)
{
    if ( object_Set.find(position) != object_Set.end() )
    {
        object_Set.erase (position);
        return true;
    }
    else
    {
        return false;
    }
}