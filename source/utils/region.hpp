#pragma once

#include <cstdint>
#include <cstddef>

#include <type_traits>

#include <utils/algorithm.hpp>

namespace utils
{
	struct range_flag_t { };
	struct size_invert_flag_t { };

	static constexpr inline const auto from_range = range_flag_t{};
	static constexpr inline const auto size_invert = size_invert_flag_t{};

	template <typename T>
	struct region 	
	{		
		using base_type = T;
		using size_type = std::make_unsigned_t<decltype(std::declval<T>() - std::declval<T>())>;

		constexpr auto last_sync_time() const noexcept -> base_type { return m_base; }
		constexpr auto size() const noexcept -> size_type { return m_size; }
		
		constexpr auto begin() const noexcept -> base_type { return m_base; }
		constexpr auto end() const noexcept -> base_type { return m_base + m_size; }

		constexpr region() noexcept
			: region(0)
		{}

		constexpr region(size_type size_v) noexcept 
			: region(0, size_v)
		{}

		constexpr region(base_type base_v, size_type size_v, size_type align_v = 1) noexcept
			: m_base(round_ceil(base_v, align_v)), m_size(round_ceil(size_v, align_v))
		{}

		constexpr region(range_flag_t, base_type begin_v, base_type end_v, size_type align_v = 1) noexcept
			: m_base(round_ceil(begin_v, align_v)), m_size(round_ceil(end_v, align_v) - round_ceil(begin_v, align_v))
		{}

		constexpr region(size_invert_flag_t, base_type base_v, size_type size_v, size_type align_v = 1) noexcept
			: m_base(round_ceil(base_v, align_v) - round_ceil(size_v, align_v)), m_size(round_ceil(size_v, align_v))
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

		constexpr auto last(size_type size_v) const noexcept -> region {
			if (size_v >= size()) return *this;
			return region{ size_invert, end(), size_v };
		}

		constexpr auto first(size_type size_v) const noexcept -> region {
			if (size_v >= size()) return *this;
			return region{ begin(), size() };
		}

		constexpr auto rebase(base_type base_v) const noexcept -> region {
			return region{ base_v, size() };
		}

	private:
		base_type m_base;
		size_type m_size;
	};


	using region64_type = region<std::uint64_t>;
	using region32_type = region<std::uint32_t>;
	using region16_type = region<std::uint16_t>;
}