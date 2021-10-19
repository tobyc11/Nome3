#pragma once

namespace tc
{

template <typename T> struct TTypeDummy
{
    static void Dummy() {}
};

template <typename T> size_t type_id() { return reinterpret_cast<size_t>(&TTypeDummy<T>::Dummy); }

}
