#pragma once

#include <type_traits>
#include <iterator>
#include <cstdint>
#include <cstddef>
#include <cassert>

namespace utils {

	template <typename _Value_type = std::uint64_t>
		requires(std::is_unsigned_v<_Value_type>)
		struct interval
	{
		using value_type = _Value_type;
		using size_type = decltype(value_type{} - value_type{});

		struct iterator_type : public std::iterator<
			std::bidirectional_iterator_tag,
			value_type,
			std::ptrdiff_t>
		{
			iterator_type(value_type base_v) noexcept
				: m_curr(base_v)
			{}

			constexpr auto operator++() noexcept -> iterator_type& { ++m_curr; return *this; }
			constexpr auto operator++(int) noexcept -> iterator_type { auto tmp = *this; ++m_curr; return tmp; }
			constexpr auto operator--() noexcept -> iterator_type& { --m_curr; return *this; }
			constexpr auto operator--(int) noexcept -> iterator_type { auto tmp = *this; --m_curr; return tmp; }
			constexpr auto operator*() const noexcept -> value_type { return m_curr; }
			constexpr auto operator==(iterator_type const& rhs_v) const noexcept -> bool { return m_curr == rhs_v.m_curr; }
			constexpr auto operator!=(iterator_type const& rhs_v) const noexcept -> bool { return m_curr != rhs_v.m_curr; }			
		private:
			value_type m_curr{ value_type{} };
		};

		constexpr interval(value_type base_v, value_type last_v) noexcept
			: m_base(base_v), m_last(last_v)
		{
			if(m_base > m_last)std::swap(m_base, m_last);
		}

		constexpr interval() noexcept = default;
		constexpr interval(interval const&) noexcept = default;
		constexpr interval(interval&&) noexcept = default;
		constexpr auto operator=(interval const&) noexcept -> interval & = default;
		constexpr auto operator=(interval&&) noexcept -> interval & = default;

		constexpr auto base() const noexcept -> value_type { return m_base; }
		constexpr auto last() const noexcept -> value_type { return m_last; }
		constexpr auto size() const noexcept -> size_type { return m_last-m_base; }		
		constexpr auto empty() const noexcept -> bool { return m_base == m_last; }

		constexpr auto cbegin() const noexcept -> iterator_type { return m_base; }
		constexpr auto begin() const noexcept -> iterator_type { return m_base; }
		constexpr auto cend() const noexcept -> iterator_type { return m_last; }
		constexpr auto end() const noexcept -> iterator_type { return m_last; }

		constexpr auto base(auto&& base_v) noexcept -> void { m_base = base_v; }
		constexpr auto last(auto&& last_v) noexcept -> void { m_last = last_v; }
		constexpr auto size(auto&& size_v) noexcept -> void { m_last = m_base + size_v; }

		constexpr auto operator==(interval const& other_v) const noexcept -> bool {
			return m_base == other_v.m_base && m_last == other_v.m_last; 
		}

		constexpr auto operator!=(interval const& other_v) const noexcept -> bool {
			return !(*this == other_v);
		}

		constexpr auto operator < (interval const& other_v) const noexcept -> bool {
			return m_base < other_v.m_base && m_last <= other_v.m_base;
		}

		constexpr auto operator > (interval const& other_v) const noexcept -> bool {
			return m_last > other_v.m_last && m_base >= other_v.m_last;
		}

		constexpr auto contains(value_type const& value_v) const noexcept -> bool {
			return m_base <= value_v && value_v < m_last;
		}

		constexpr auto overlaps(interval const& other_v) const noexcept -> bool {
			return contains(other_v.base()) || contains(other_v.last() - 1u);
		}

		constexpr auto envelops(interval const& other_v) const noexcept -> bool {
			return contains(other_v.base()) && contains(other_v.last() - 1u);		
		}

		constexpr auto merge(interval const& other_v) const noexcept -> interval {
			assert (overlaps(other_v));
			return { std::min(m_base, other_v.m_base), 
				       std::max(m_last, other_v.m_last) };
		}

		constexpr auto split_by(value_type value_v) const noexcept 
			-> std::pair<interval, interval> 
		{
			return { {m_base, value_v}, {value_v, m_last} };
		}

		constexpr auto split_by(interval const& other_v) const noexcept 
			-> std::pair<interval, interval> 
		{
			return { {m_base, other_v.m_base}, {other_v.m_last, m_last} };
		}

	private:
		value_type m_base{ value_type{} };
		value_type m_last{ value_type{} };
	};


	struct less_compare_interval_base 
	{
		template <typename _Value_type>
	  constexpr auto operator()(interval<_Value_type> const& lhs, 
			                        interval<_Value_type> const& rhs) 
			const noexcept -> bool
		{
			return lhs.base() < rhs.base();
		}
	};

	struct less_compare_interval_size 
	{
		template <typename _Value_type, typename _Size>
		constexpr auto operator()(interval<_Value_type> const& lhs, 
			                        interval<_Value_type> const& rhs) 
			const noexcept -> bool 
		{
			return lhs.size() < rhs.size();
		}
	};

	struct less_compare_interval_last
	{
		template <typename _Value_type, typename _Size>
		constexpr auto operator()(interval<_Value_type> const& lhs,
			                        interval<_Value_type> const& rhs)
			const noexcept -> bool
		{
			return lhs.last() < rhs.last();
		}
	};

}