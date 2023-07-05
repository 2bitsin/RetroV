#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <tuple>

namespace utils
{
	namespace detail
	{
		template <typename KeyType>
		using interval_type = std::tuple<KeyType, KeyType>;


		template <typename K>
		auto rhs (interval_type<K> const& interval_v) -> decltype(auto) { 
			auto &[_, rhs_v] = interval_v; return rhs_v; 
		}

		template <typename K>
		auto lhs (interval_type<K> const& interval_v) -> decltype(auto) {
			auto &[lhs_v, _] = interval_v; return lhs_v; 
		}

		template <typename K>
		auto rhs(interval_type<K>& interval_v) -> auto&& {
			auto& [_, rhs_v] = interval_v; return rhs_v;
		}

		template <typename K>
		auto lhs(interval_type<K>& interval_v) -> auto&& {
			auto& [lhs_v, _] = interval_v; return lhs_v;
		}
	}

	template <typename KeyType, typename ValueType>
	struct interval_map 
	{
		using key_type = detail::interval_type<KeyType>;
		using mapped_type = ValueType;
		using value_type = std::pair<key_type, mapped_type>;

		auto insert(key_type const& key_v, mapped_type const& val_v) -> bool 
		{
			auto lower_b = std::lower_bound(
				m_intervals.begin(), m_intervals.end(), value_type{key_v, val_v},
				[](auto&& lhs_v, auto&& rhs_v) {
					auto&& [llhs_v, _0] = lhs_v.first;
					auto&& [rlhs_v, _1] = rhs_v.first;

					return llhs_v < rlhs_v;
				});
		
			if (m_intervals.end() == lower_b || 
				detail::rhs(key_v) <= detail::lhs(lower_b->first)) {
				auto where_v = m_intervals.emplace(lower_b, key_v, val_v);
				if (where_v != m_intervals.begin()) {
					auto prev_v = std::prev(where_v);
					if (detail::rhs(prev_v->first) >= detail::lhs(where_v->first)) {
						detail::rhs(prev_v->first) = detail::lhs(where_v->first);						
					}
				}
				return true;
			}

			auto last_b = lower_b;
			while(lower_b != m_intervals.end() && 
				detail::rhs(key_v) >= detail::rhs(lower_b->first)) {
				std::advance(lower_b, 1u);
			}

			if (lower_b != m_intervals.end() && 
				detail::rhs(key_v) >= detail::lhs(lower_b->first)) {
				detail::lhs(lower_b->first) = detail::rhs(key_v);
			}
	
			lower_b =	m_intervals.erase(last_b, lower_b);
			auto where_v = m_intervals.emplace(lower_b, key_v, val_v);
			if (where_v != m_intervals.begin()) {
				auto prev_v = std::prev(where_v);
				if (detail::rhs(prev_v->first) >= detail::lhs(where_v->first)) {
					detail::rhs(prev_v->first) = detail::lhs(where_v->first);
				}
			}
			return true;
		}

	protected:
		
	private:
		std::vector<value_type> m_intervals;
	};

}