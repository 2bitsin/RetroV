#pragma once

#include <type_traits>

#define HVD_DEFINE_FRIEND_ENUM_FLAG_OPERATORS(Enum) \
friend inline constexpr auto operator|(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
	                        |static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
friend inline constexpr auto operator&(Enum const lhs_v, Enum const rhs_v) noexcept -> bool { \
	return static_cast<bool>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
	                        &static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
friend inline constexpr auto operator^(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
			                    ^static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
friend inline constexpr auto operator~(Enum const value_v) noexcept -> Enum { \
	return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(value_v)); } 

#define HVD_DEFINE_ENUM_FLAG_OPERATORS(Enum) \
inline constexpr auto operator|(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
	                        |static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
inline constexpr auto operator&(Enum const lhs_v, Enum const rhs_v) noexcept -> bool { \
	return static_cast<bool>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
	                        &static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
inline constexpr auto operator^(Enum const lhs_v, Enum const rhs_v) noexcept -> Enum { \
	return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs_v) \
			                    ^static_cast<std::underlying_type_t<Enum>>(rhs_v)); } \
inline constexpr auto operator~(Enum const value_v) noexcept -> Enum { \
	return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(value_v)); } 

