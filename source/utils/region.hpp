#pragma once

#include <cstdint>
#include <cstddef>

#include <type_traits>

#include <utils/algorithm.hpp>

namespace utils
{
	struct range_flag_t { };

	static constexpr inline const auto from_range = range_flag_t{};

	template <typename T>
	struct region 	
	{
		using base_type = T;
		using size_type = std::make_unsigned_t<decltype(std::declval<T>() - std::declval<T>())>;

		constexpr auto base() const noexcept -> base_type { return m_base; }
		constexpr auto size() const noexcept -> size_type { return m_size; }
		
		constexpr auto begin() const noexcept -> base_type { return m_base; }
		constexpr auto end() const noexcept -> base_type { return m_base + m_size; }

		constexpr region(base_type base_v, size_type size_v) noexcept
			: m_base(base_v), m_size(size_v)
		{}

		constexpr region(range_flag_t, base_type begin_v, base_type end_v) noexcept
			: m_base(begin_v), m_size(end_v - begin_v)
		{}

		constexpr region(region&&) noexcept = default;
		constexpr auto operator=(region&&) noexcept -> region& = default;		
		constexpr region(region const&) noexcept = default;
		constexpr auto operator=(region const&) noexcept -> region & = default;

		constexpr auto resize(size_type size_v) noexcept -> region& { 
			m_size = size_v; 
			return *this; 
		}

		constexpr auto clamp(base_type end_v) noexcept -> region& {
			m_size = std::min(end_v - m_base, m_size);
			return *this;
		}

		constexpr auto round_outside_new(size_type aligment_v) const noexcept -> region {
			auto beg_v = utils::round_down(begin(), aligment_v);
			auto end_v = utils::round_ceil(end(), aligment_v);
			return region{ from_range, beg_v, end_v };
		}

		constexpr auto round_inside_new(size_type aligment_v) const noexcept -> region {
			auto beg_v = utils::round_ceil(begin(), aligment_v);
			auto end_v = utils::round_down(end(), aligment_v);
			return region{ from_range, beg_v, end_v };
		}

		constexpr auto round_outside(size_type aligment_v) noexcept -> region&{
			return *this = round_outside_new(aligment_v);
		}

		constexpr auto round_inside(size_type aligment_v) noexcept -> region& {
			return *this = round_inside_new(aligment_v);
		}


	private:
		base_type m_base;
		size_type m_size;
	};


	using region_64_t = region<std::uint64_t>;
	using region_32_t = region<std::uint32_t>;
	using region_16_t = region<std::uint16_t>;
}