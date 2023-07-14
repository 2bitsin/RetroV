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
	
		auto upper_bound(auto&& begin, auto&& end, point_type const& point_v) const
		{
			return std::upper_bound(begin, end, point_v, 
				[](auto&& point_v, auto&& node_v) {
					return point_v < node_v.second;
				});
		}

		auto lower_bound(auto&& begin, auto&& end, point_type const& point_v) const
		{
			return std::lower_bound(begin, end, point_v, 
				[](auto&& node_v, auto&& point_v) {
					return node_v.second < point_v;
				});
		}

		interval_map(value_type defval_v = value_type())
			: m_intervals{node_type(std::move(defval_v), point_type(0))}
		{}

		auto insert(interval_type const& bounds_v, value_type value_v) -> void
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

		auto at (point_type const& point_v) const -> value_type
		{
			auto [value_v, _] = *std::prev(upper_bound(
				m_intervals.begin(), m_intervals.end(), point_v));
			return value_v;
		}

	private:
		std::vector<node_type> m_intervals;
	};

}