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

template <typename enum_t>
inline std::enable_if_t<std::is_enum_v<enum_t>, bool> Any(enum_t flags)
{
	return TO_UNDERLYING(enum_t, flags) != 0;
}

template <typename enum_t>
inline std::enable_if_t<std::is_enum_v<enum_t>, bool> Any(enum_t flags, enum_t compare)
{
	return (TO_UNDERLYING(enum_t, flags) & TO_UNDERLYING(enum_t, compare)) != 0;
}

#undef TO_UNDERLYING
