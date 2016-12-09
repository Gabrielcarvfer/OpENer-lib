//
// Created by gabriel on 18/11/2016.
//

#include "CIP_Template.h"

template <typename T>
T * CIP_Template<T>::GetInstance(CipUdint instance_number)
{
    if (CIP_Template<T>::object_Set.size() >= instance_number)
        return CIP_Template<T>::object_Set[instance_number];
    else
        return NULL;
}

template <typename T>
T * CIP_Template<T>::GetClass()
{
    GetInstance(0);
}

template <typename T>
CipUdint CIP_Template<T>::GetNumberOfInstances()
{
    return CIP_Template<T>::object_Set.size();
}

template <typename T>
CipUdint CIP_Template<T>::GetInstanceNumber(T * instance)
{
    return std::distance(CIP_Template<T>::object_Set.begin(), CIP_Template<T>::object_Set.find(instance) );
}

template <typename T>
bool CIP_Template<T>::AddClassInstance(T* instance, CipUdint position)
{
    CIP_Template<T>::object_Set.emplace(position,instance);
    auto it = CIP_Template<T>::object_Set.find(position);
    return (it != CIP_Template<T>::object_Set.end());
}

template <typename T>
bool CIP_Template<T>::RemoveClassInstance(T* instance)
{
    for (auto it = CIP_Template<T>::object_Set.begin(); it != CIP_Template<T>::object_Set.end(); it++)
    {
        if (it->second == instance)
        {
            CIP_Template<T>::object_Set.erase(it);
            return true;
        }
    }
    return false;
}

template <typename T>
bool CIP_Template<T>::RemoveClassInstance(CipUdint position)
{
    if ( CIP_Template<T>::object_Set.find(position) != CIP_Template<T>::object_Set.end() )
    {
        CIP_Template<T>::object_Set.erase (position);
        return true;
    }
    else
    {
        return false;
    }
}