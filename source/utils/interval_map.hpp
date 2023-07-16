#pragma once

#include <algorithm>
#include <stdexcept>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>

namespace utils
{

	template <typename PointType, typename ValueType>
	struct interval_map 
	{		
		using point_type = PointType;
		using value_type = ValueType;
		using interval_type = std::pair<point_type, point_type>;
		using node_type = std::pair<value_type, point_type>;	

	protected:
		inline auto upper_bound(auto&& begin, auto&& end, point_type const& point_v) const
		{
			return std::upper_bound(begin, end, point_v, 
				[](auto&& point_v, auto&& node_v) {
					return point_v < node_v.second;
				});
		}

		inline auto lower_bound(auto&& begin, auto&& end, point_type const& point_v) const
		{
			return std::lower_bound(begin, end, point_v, 
				[](auto&& node_v, auto&& point_v) {
					return node_v.second < point_v;
				});
		}

	public:
		inline interval_map(value_type defval_v = value_type())
			: m_intervals{node_type(std::move(defval_v), point_type(0))}
		{}

		inline auto insert(interval_type const& bounds_v, value_type value_v) -> void
		{
			auto [base_v, last_v] = bounds_v;
			if (base_v >= last_v) throw std::invalid_argument("bounds_v.first >= bounds_v.second");
			auto last_pos_v = upper_bound(m_intervals.begin(), m_intervals.end(), last_v);			
			auto pre_last_pos_v = std::prev(last_pos_v);
			last_pos_v = m_intervals.emplace(last_pos_v, pre_last_pos_v->first, last_v);
			auto base_pos_v = lower_bound(m_intervals.begin(), m_intervals.end(), base_v);
			last_pos_v = m_intervals.erase(base_pos_v, last_pos_v);
			m_intervals.emplace(last_pos_v, std::move(value_v), base_v);
		}

		inline auto lower_bound(point_type const& point_v) const { return lower_bound(m_intervals.begin(), m_intervals.end(), point_v); };
		inline auto upper_bound(point_type const& point_v) const { return upper_bound(m_intervals.begin(), m_intervals.end(), point_v); };

		inline auto value_at (point_type const& point_v) const -> value_type
		{
			auto [value_v, _] = find(point_v);
			return value_v;
		}
		
		inline auto find(point_type const& point_v) const -> std::tuple<value_type, point_type>
		{
			return *std::prev(upper_bound(m_intervals.begin(), m_intervals.end(), point_v));
		}

		struct const_iterator 
		{
			inline const_iterator(std::vector<node_type> const& intervals_v, std::vector<node_type>::const_iterator curr_v)
				: m_intervals{intervals_v}, m_curr{curr_v}
			{}

			inline const_iterator(std::vector<node_type> const& intervals_v)
				: m_intervals{intervals_v}, m_curr{intervals_v.end()}
			{}

			inline auto operator++() -> const_iterator&
			{
				++m_curr;
				return *this;
			}

			inline auto operator++(int) -> const_iterator
			{
				auto copy_v = *this;
				++m_curr;
				return copy_v;
			}

			inline auto operator--() -> const_iterator&
			{
				--m_curr;
				return *this;
			}

			inline auto operator--(int) -> const_iterator
			{
				auto copy_v = *this;
				--m_curr;
				return copy_v;
			}

			inline auto operator==(const_iterator const& other_v) const -> bool
			{
				return m_curr == other_v.m_curr;
			}

			inline auto operator!=(const_iterator const& other_v) const -> bool
			{
				return m_curr != other_v.m_curr;
			}
			
			inline auto operator*() const
				-> std::tuple<point_type, point_type, value_type> 
			{
				if (m_curr == m_intervals.end()) 
				{	throw std::out_of_range("iterator out of range");	}

				if (auto next_v = std::next(m_curr); next_v != m_intervals.end()) 
				{ auto [_______, last_v] = *next_v;
					auto [value_v, base_v] = *m_curr;
					return{ base_v, last_v, value_v }; }

				return{ m_curr->second, m_curr->second, m_curr->first };
			}

		private:
			std::vector<node_type> const& m_intervals;
			std::vector<node_type>::const_iterator m_curr;

		};

		auto begin() const -> const_iterator {
			return{ m_intervals, m_intervals.begin() };
		}

		auto end() const -> const_iterator {
			return{ m_intervals };
		}

	private:
		std::vector<node_type> m_intervals;
	};

}