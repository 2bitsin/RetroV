#pragma once

#include <cstdint>
#include <cstddef>

#include <compare>

#include <type_traits>

namespace utils
{
	namespace detail 
	{
		static inline constexpr auto compare(auto const& lhs_v, auto const& rhs_v) -> std::strong_ordering 
		{
			auto lhs_size_v = lhs_v.size();
			auto rhs_size_v = rhs_v.size();
			for (auto i = 0u; true; i += 1u)
			{
				if (i < lhs_size_v && i < rhs_size_v) {
					if (lhs_v[i] < rhs_v[i]) 
						return std::strong_ordering::less;
					if (lhs_v[i] > rhs_v[i]) 
						return std::strong_ordering::greater;
					continue;
				}
				if (i < rhs_size_v) {
					return std::strong_ordering::less;
				}
				if (i < lhs_size_v) {
					return std::strong_ordering::greater;
				}				
				return std::strong_ordering::equal;
			}
		}
	}

	template <typename T, size_t N>
	struct pattern
	{
		constexpr inline auto size() const noexcept -> size_t { return N; }

		template <typename... V>
		requires (std::is_trivially_constructible_v<T, V> 
			&& (std::is_same_v<T, V> && ...))
		constexpr pattern(V ... vn) 			
			: value { vn... }
		{}			

		constexpr pattern(T const (&init_v) [N])
		{
			for(auto i = 0u; i < N; i += 1u)
				value[i] = init_v[i];
		}

		constexpr auto operator [] (size_t index_v) const -> T {
			return value[index_v];
		}

		template <typename Q, size_t N>
		requires (std::three_way_comparable_with<T, Q>)
		friend inline constexpr auto operator <=> (pattern const& lhs_v, std::span<Q const> rhs_v)noexcept -> std::strong_ordering {
			return detail::compare(lhs_v, rhs_v);		
		}

		template <typename Q, size_t N>
		requires (std::three_way_comparable_with<T, Q>)
		friend inline constexpr auto operator <=> (std::span<Q const> lhs_v, pattern const& rhs_v)noexcept -> std::strong_ordering {
			return detail::compare(lhs_v, rhs_v);
		}

		template <typename Q, size_t M>
		requires (std::three_way_comparable_with<T, Q>)		
		friend inline constexpr auto operator <=> (pattern<T, N> const& lhs_v, pattern<Q, M> const& rhs_v)noexcept -> std::strong_ordering  {
			return detail::compare(lhs_v, rhs_v);
		}

		T value[N];
	};



	template <typename... V>	
	pattern(V...) -> pattern<std::common_type_t<V...>, sizeof...(V)>;
}