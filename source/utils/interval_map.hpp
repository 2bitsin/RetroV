#pragma once

#include <utils/interval.hpp>

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
		using interval_type = interval<point_type>;
		using node_type = std::pair<interval_type, value_type>;

		auto insert(interval_type new_bounds_v, value_type new_value_v) -> bool
		{
			// Perform clipping
			if (m_intervals.empty() || new_bounds_v > m_intervals.back().first) {
				m_intervals.emplace_back(new_bounds_v, new_value_v);
				return true;
			} else if (new_bounds_v<m_intervals.front().first) {
				m_intervals.emplace(m_intervals.begin(), new_bounds_v, new_value_v);
				return true;	
			}

			static const auto constexpr cmp_b=[](auto&&a,auto&&b){return a.first.base()<b.base();};
		 	auto where_v = std::lower_bound(m_intervals.begin(), m_intervals.end(), new_bounds_v, cmp_b);
			if (where_v==m_intervals.end()||new_bounds_v<where_v->first){
				m_intervals.emplace(where_v, new_bounds_v, new_value_v);
				return true;
			}

			__debugbreak();


			std::vector<node_type> clipped_v;
			auto inserted_v{ false };
			clipped_v.reserve(m_intervals.size() + 2u);
			for (auto const& [cur_bounds_v, cur_value_v] : m_intervals) 
			{
				if (!cur_bounds_v.overlaps(new_bounds_v))
				{ 
					if (cur_bounds_v > new_bounds_v && !inserted_v) {
						clipped_v.emplace_back(new_bounds_v, new_value_v);
						inserted_v = true;
					}
					clipped_v.emplace_back(cur_bounds_v, cur_value_v);					
					continue; 
				}

				if (cur_bounds_v == new_bounds_v) 
				{
					clipped_v.emplace_back(cur_bounds_v, new_value_v);
					inserted_v = true;
					continue;
				}

				if (new_bounds_v.envelops(cur_bounds_v))
				{
					clipped_v.emplace_back(cur_bounds_v, new_value_v);
					inserted_v = true;
					continue;
				}

				if (cur_bounds_v.envelops(new_bounds_v))  					
				{ 
					auto [lhsb_v, rhsb_v] = cur_bounds_v.split_by(new_bounds_v);
					if (!lhsb_v.empty()) clipped_v.emplace_back(lhsb_v, cur_value_v);
					clipped_v.emplace_back(new_bounds_v, new_value_v);
					if (!rhsb_v.empty()) clipped_v.emplace_back(rhsb_v, cur_value_v);
					inserted_v = true;
					continue;
				}

				if (cur_bounds_v.contains(new_bounds_v.base())) 
				{
					auto [oldb_v, newb_v] = cur_bounds_v.split_by(new_bounds_v.base());
					clipped_v.emplace_back(oldb_v, cur_value_v);
					clipped_v.emplace_back(newb_v, new_value_v);
					inserted_v = true;
					continue;
				}

				if (cur_bounds_v.contains(new_bounds_v.last())) 
				{
					auto [newb_v, oldb_v] = cur_bounds_v.split_by(new_bounds_v.last());
					clipped_v.emplace_back(newb_v, new_value_v);
					clipped_v.emplace_back(oldb_v, cur_value_v);
					inserted_v = true;
					continue;
				}
			}

			m_intervals.clear();

			for (auto const& [cur_bounds_v, cur_value_v] : clipped_v) 
			{
				if (!new_bounds_v.envelops(cur_bounds_v)) 
				{
					m_intervals.emplace_back(cur_bounds_v, cur_value_v);
					continue;
				}
				if (!m_intervals.empty()) 
				{
					auto& last_bounds_v = m_intervals.back().first;
					last_bounds_v = last_bounds_v.merge(cur_bounds_v);
					continue;
				}
				m_intervals.emplace_back(cur_bounds_v, cur_value_v);
			}
			return true;
		}
		
		std::vector<node_type> m_intervals;
		
	};

}