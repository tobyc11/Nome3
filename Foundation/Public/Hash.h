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

template <typename T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} /* namespace tc */
