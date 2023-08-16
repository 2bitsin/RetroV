#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <utils/algorithm.hpp>

#include <algorithm>
#include <tuple>

namespace win32
{

	template <std::size_t _Size>
	struct WHvRegistersStatic
	{
		static inline constexpr const auto Size = _Size;

		constexpr WHvRegistersStatic(std::pair<WHV_REGISTER_NAME, WHV_REGISTER_VALUE> const (&init_v)[_Size])
		{
			for (auto index_v = 0u; index_v < Size; index_v += 1u) 
			{
				auto const [names_v, value_v] = init_v[index_v];
				insert(names_v, value_v);
			}
		}		

		constexpr auto insert(WHV_REGISTER_NAME names_v, WHV_REGISTER_VALUE value_v) -> std::size_t 
		{
			auto const names_beg_v = std::begin(m_Names);
			auto const names_end_v = std::next(std::begin(m_Names), m_Size);
			auto const value_beg_v = std::begin(m_Value);
			auto const value_end_v = std::next(std::begin(m_Value), m_Size);

			auto const position_v = utils::upper_bound(names_beg_v, names_end_v, names_v);
			if (names_end_v == position_v) {

				if (m_Size >= Size) {
					throw std::out_of_range("Overflow");
				}

				m_Names[m_Size] = names_v;
				m_Value[m_Size] = value_v;
				m_Size += 1u;
				return m_Size - 1u;
			}

			auto const offset_v = std::distance(names_beg_v, position_v);
			if (offset_v > 0u && names_v == m_Names[offset_v - 1u]) {
				m_Value[offset_v - 1u] = value_v;
				return offset_v - 1u;
			}

			if (m_Size >= Size) {
				throw std::out_of_range("Overflow");
			}
			m_Size += 1u;

			std::shift_right(std::next(names_beg_v, offset_v), std::next(names_end_v, 1u), 1u);
			std::shift_right(std::next(value_beg_v, offset_v), std::next(value_end_v, 1u), 1u);

			m_Names[offset_v] = names_v;
			m_Value[offset_v] = value_v;

			return offset_v;
		}

		constexpr auto operator [] (WHV_REGISTER_NAME index_v) const -> WHV_REGISTER_VALUE
		{
			auto const names_beg_v = std::begin(m_Names);
			auto const names_end_v = std::next(std::begin(m_Names), m_Size);
			auto const position_v = utils::lower_bound(names_beg_v, names_end_v, index_v);
			if (names_end_v == position_v) {
				throw std::out_of_range("Not found");
			}
			auto const offset_v = std::distance(names_beg_v, position_v);
			if (index_v != m_Names[offset_v]) {
				throw std::out_of_range("Not found");
			}
			return m_Value[offset_v];			
		}

		inline auto ApplyTo(win32::WHvProcessor const& processor_v) const -> std::int32_t {
			return processor_v.SetRegisters({ m_Names, m_Size }, { m_Value, m_Size });
		}

	private:
		std::size_t m_Size { 0u };
		WHV_REGISTER_NAME  m_Names [Size] { };
		WHV_REGISTER_VALUE m_Value [Size] { };
	};

	template <std::size_t _Size>
	WHvRegistersStatic(std::pair<WHV_REGISTER_NAME, WHV_REGISTER_VALUE> const (&init_v)[_Size]) 
		-> WHvRegistersStatic<_Size>;


	static inline constexpr auto GetInitialProcessorState() 
	{
		const win32::WHvRegistersStatic state_v
		({
			{ WHvX64RegisterCs,     { .Segment = { .Base = 0xF0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } } },
			{ WHvX64RegisterEs,     { .Segment = { .Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } } },
			{ WHvX64RegisterDs,     { .Segment = { .Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } } },
			{ WHvX64RegisterFs,     { .Segment = { .Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } } },
			{ WHvX64RegisterGs,     { .Segment = { .Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } } },
			{ WHvX64RegisterSs,     { .Segment = { .Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } } },
			{ WHvX64RegisterIdtr,   { .Table = { .Limit = 0x03FFu, .Base = 0x00000000u  } } },
			{ WHvX64RegisterGdtr,   { .Table = { .Limit = 0x0000u, .Base = 0x00000000u  } } },
			{ WHvX64RegisterRflags, { .Reg64 = 0x0000'0000'0000'0002u } },
			{ WHvX64RegisterRip,    { .Reg64 = 0x0000'0000'0000'FFF0u } },
			{ WHvX64RegisterRbx,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRcx,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRdx,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRsi,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRdi,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRbp,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterRsp,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR8,     { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR9,     { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR10,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR11,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR12,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR13,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR14,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterR15,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterCr0,    { .Reg64 = 0x0000'0000'6000'0010u } },
			{ WHvX64RegisterCr2,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterCr3,    { .Reg64 = 0x0000'0000'0000'0000u } },
			{ WHvX64RegisterCr4,    { .Reg64 = 0x0000'0000'0000'0000u } }
		});
		return state_v;
	}
}