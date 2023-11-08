#pragma once

#include <cstdint>
#include <cstddef>

#include <compare>

#include <type_traits>

namespace utils
{
	template <typename T, std::size_t N>
	struct pattern
	{

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

		constexpr auto operator <=> (std::span<T const> what_v) -> std::strong_ordering
			for(auto i=0u; i<what_v.size()&&i<N; i+=1u) {
				if (what_v[i] == value[i])
					continue;				
			}
		}
	
		T value[N];
	};
}