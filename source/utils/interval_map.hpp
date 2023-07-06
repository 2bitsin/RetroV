#pragma once

#include <algorithm>
#include <stdexcept>
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
		using point_type = KeyType;
		using key_type = detail::interval_type<point_type>;
		using mapped_type = ValueType;
		using value_type = std::pair<key_type, mapped_type>;
		using iterator_type = typename std::vector<value_type>::iterator;
		using const_iterator_type = typename std::vector<value_type>::const_iterator;

		inline auto insert(key_type const& key_v, mapped_type const& val_v)
			-> std::pair<bool, iterator_type>
		{
			if (detail::lhs(key_v) >= detail::rhs(key_v)) {
				std::invalid_argument("Interval size must be positive and non-zero");
				return{ false, m_intervals.end() };
			}

			bool has_overlapped_v = false;

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
						has_overlapped_v = true;
					}
				}
				return{ !has_overlapped_v, where_v };
			}

			auto last_b = lower_b;
			while(lower_b != m_intervals.end() && 
				detail::rhs(key_v) >= detail::rhs(lower_b->first)) {
				std::advance(lower_b, 1u);
				has_overlapped_v = true;
			}

			if (lower_b != m_intervals.end() && 
				detail::rhs(key_v) >= detail::lhs(lower_b->first)) {
				detail::lhs(lower_b->first) = detail::rhs(key_v);
				has_overlapped_v = true;
			}
	
			lower_b =	m_intervals.erase(last_b, lower_b);
			auto where_v = m_intervals.emplace(lower_b, key_v, val_v);
			if (where_v != m_intervals.begin()) {
				auto prev_v = std::prev(where_v);
				if (detail::rhs(prev_v->first) >= detail::lhs(where_v->first)) {
					detail::rhs(prev_v->first) = detail::lhs(where_v->first);
					has_overlapped_v = true;
				}
			}
			return{ !has_overlapped_v, where_v };
		}

		inline auto at(point_type const& point_v) const -> mapped_type const& {
			if (auto pos_v=find(point_v); m_intervals.end()!=pos_v)
				return pos_v->second;
			throw std::out_of_range("No such point in interval map");
		}

		inline auto at(point_type const& point_v) -> mapped_type& {
			if (auto pos_v=find(point_v); m_intervals.end()!=pos_v) 
				return pos_v->second;
			throw std::out_of_range("No such point in interval map");
		}

		inline auto at(point_type const& point_v, mapped_type const& default_v) const -> mapped_type const& {
			if (auto pos_v=find(point_v); m_intervals.end()!=pos_v) 
				return pos_v->second;
			return default_v;
		}

		inline auto contains(point_type const& point_v) const -> bool {
			return m_intervals.end() != find(point_v);			
		}

		inline auto find (point_type const& point_v) const -> const_iterator_type
		{
			auto upper_b = std::upper_bound(
				m_intervals.begin(), m_intervals.end(), value_type{ key_type{point_v, point_v}, mapped_type{} },
				[](auto&& lhs_v, auto&& rhs_v) {
					auto&& [_0, lrhs_v] = lhs_v.first;
					auto&& [_1, rrhs_v] = rhs_v.first;
					return lrhs_v < rrhs_v;
				});

			if (m_intervals.end() == upper_b || detail::rhs(upper_b->first) <= point_v || detail::lhs(upper_b->first) > point_v) {
				return m_intervals.end();
			}
			return upper_b;
		}

		template <typename... T> inline auto erase(T&&... args_v) -> auto {
			return m_intervals.erase(std::forward<T>(args_v)...); }

		inline auto begin() -> decltype(auto) { return m_intervals.begin(); }
		inline auto begin() const -> decltype(auto) { return m_intervals.begin(); }
		inline auto end() -> decltype(auto) { return m_intervals.end(); }
		inline auto end() const -> decltype(auto) { return m_intervals.end(); }
		inline auto rbegin() -> decltype(auto) { return m_intervals.rbegin(); }
		inline auto rbegin() const -> decltype(auto) { return m_intervals.rbegin(); }
		inline auto rend() -> decltype(auto) { return m_intervals.rend(); }
		inline auto rend() const -> decltype(auto) { return m_intervals.rend(); }
		inline auto cbegin() const -> decltype(auto) { return m_intervals.cbegin(); }
		inline auto cend() const -> decltype(auto) { return m_intervals.cend(); }
		inline auto size() const -> decltype(auto) { return m_intervals.size(); }
		inline auto empty() const -> decltype(auto) { return m_intervals.empty(); }

		inline auto operator [] (point_type const& point_v) const -> mapped_type const& { return at(point_v); }
		inline auto operator [] (point_type const& point_v) -> mapped_type& { return at(point_v); }

		inline auto swap (interval_map& other_v) -> void {
			m_intervals.swap(other_v.m_intervals); 
		}

	protected:
		inline auto find(point_type const& point_v) -> iterator_type
		{
			auto upper_b = std::upper_bound(
				m_intervals.begin(), m_intervals.end(), value_type{ key_type{point_v, point_v}, mapped_type{} },
				[](auto&& lhs_v, auto&& rhs_v) {
					auto&& [_0, lrhs_v] = lhs_v.first;
					auto&& [_1, rrhs_v] = rhs_v.first;
					return lrhs_v < rrhs_v;
				});

			if (m_intervals.end() == upper_b || detail::rhs(upper_b->first) <= point_v || detail::lhs(upper_b->first) > point_v) {
				return m_intervals.end();
			}
			return upper_b;
		}

	private:
		std::vector<value_type> m_intervals;
	};

}