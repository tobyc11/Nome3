#pragma once
#include <utility>
#include <functional>
#include <type_traits>

namespace tc
{

template <typename T>
struct hash
{
public:
    std::size_t operator()(T const& v) const
    {
        return hash_value(v);
    }
};

template<> struct hash<bool> : public std::hash<bool> {};
template<> struct hash<char> : public std::hash<char> {};
template<> struct hash<signed char> : public std::hash<signed char> {};
template<> struct hash<unsigned char> : public std::hash<unsigned char> {};
template<> struct hash<char16_t> : public std::hash<char16_t> {};
template<> struct hash<char32_t> : public std::hash<char32_t> {};
template<> struct hash<wchar_t> : public std::hash<wchar_t> {};
template<> struct hash<short> : public std::hash<short> {};
template<> struct hash<unsigned short> : public std::hash<unsigned short> {};
template<> struct hash<int> : public std::hash<int> {};
template<> struct hash<unsigned int> : public std::hash<unsigned int> {};
template<> struct hash<long> : public std::hash<long> {};
template<> struct hash<long long> : public std::hash<long long> {};
template<> struct hash<unsigned long> : public std::hash<unsigned long> {};
template<> struct hash<unsigned long long> : public std::hash<unsigned long long> {};
template<> struct hash<float> : public std::hash<float> {};
template<> struct hash<double> : public std::hash<double> {};
template<> struct hash<long double> : public std::hash<long double> {};
template<class T> struct hash<T*> : public std::hash<T*> {};

template <typename T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= tc::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} /* namespace tc */
