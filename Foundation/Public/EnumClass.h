#pragma once
#include <type_traits>

#define DEFINE_ENUM_CLASS_BITWISE_OPERATORS(Enum)\
inline Enum& operator|=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type_t<Enum>)lhs | (std::underlying_type_t<Enum>)rhs); }\
inline Enum& operator&=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type_t<Enum>)lhs & (std::underlying_type_t<Enum>)rhs); }\
inline Enum& operator^=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type_t<Enum>)lhs ^ (std::underlying_type_t<Enum>)rhs); }\
inline constexpr Enum operator|(Enum lhs, Enum rhs) { return (Enum)((std::underlying_type_t<Enum>)lhs | (std::underlying_type_t<Enum>)rhs); }\
inline constexpr Enum operator&(Enum lhs, Enum rhs) { return (Enum)((std::underlying_type_t<Enum>)lhs & (std::underlying_type_t<Enum>)rhs); }\
inline constexpr Enum operator^(Enum lhs, Enum rhs) { return (Enum)((std::underlying_type_t<Enum>)lhs ^ (std::underlying_type_t<Enum>)rhs); }\
inline constexpr bool operator!(Enum e) { return !(std::underlying_type_t<Enum>)e; }\
inline constexpr Enum operator~(Enum e) { return (Enum)~(std::underlying_type_t<Enum>)e; }

#define TO_UNDERLYING(T, x) static_cast<typename std::underlying_type<T>::type>(x)

// https://stackoverflow.com/questions/15586163/c11-type-trait-to-differentiate-between-enum-class-and-regular-enum
template <typename E>
using is_scoped_enum = std::integral_constant<
    bool,
    std::is_enum<E>::value && !std::is_convertible<E, int>::value>;

template<typename E>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<E>::value;

template <typename enum_t>
inline std::enable_if_t<is_scoped_enum_v<enum_t>, bool> Any(enum_t flags)
{
	return TO_UNDERLYING(enum_t, flags) != 0;
}

template <typename enum_t>
inline std::enable_if_t<is_scoped_enum_v<enum_t>, bool> Any(enum_t flags, enum_t compare)
{
	return (TO_UNDERLYING(enum_t, flags) & TO_UNDERLYING(enum_t, compare)) != 0;
}

#undef TO_UNDERLYING
