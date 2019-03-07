#pragma once

namespace tc
{

template <typename T> struct hash;

template <typename T>
struct hash
{
public:
    std::size_t operator()(T const& v) const
    {
        return hash_value(v);
    }
};

} /* namespace tc */
