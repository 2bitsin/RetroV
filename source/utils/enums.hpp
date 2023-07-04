#pragma once

#include <type_traits>

#define ENUM_DEFINE_OPERATORS(Enum) \
static inline constexpr auto operator|(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v); \
	                        |static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
static inline constexpr auto operator&(Enum const lhs_v, Enum const rhs_v) noexcept -> bool { \
	return static_cast<bool>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
	                        &static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
static inline constexpr auto operator^(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
			                    ^static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
static inline constexpr auto operator~(Enum const value_v) noexcept -> Enum { \
	return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(value_v)); } 
