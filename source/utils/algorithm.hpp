#pragma once

#include <type_traits>
#include <concepts>
#include <iterator>
#include <string>
#include <array>

namespace utils
{

  template <typename Iterator, typename T>
	static inline constexpr auto upper_bound(Iterator first, Iterator last, const T& value) -> Iterator
  {
    Iterator it;
    typename std::iterator_traits<Iterator>::difference_type count, step;
    count = std::distance(first, last);
    while (count > 0)
    {
      it = first; 
      step = count / 2; 
      std::advance(it, step);
        if (!(value < *it))
        {
          first = ++it;
          count -= step + 1;
        } 
        else
          count = step;
    }
    return first;
  }

	template <typename Iterator, typename T>
	static inline constexpr auto lower_bound(Iterator first, Iterator last, const T& value) -> Iterator
	{
		Iterator it;
		typename std::iterator_traits<Iterator>::difference_type count, step;
		count = std::distance(first, last);

		while (count > 0)
		{
			it = first;
			step = count / 2;
			std::advance(it, step);

			if (*it < value)
			{
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}

		return first;
	}


	
	static inline auto bitset_to_string(std::integral auto bits) -> std::string
	{
		std::string result_v;
		for (auto i = 0u; i < 8u * sizeof(bits); i += 1u) {
			if (bits & 1u) {
				if (!result_v.empty())
					result_v += ", ";
				result_v += std::to_string(i);
			}
			bits >>= 1u;
		}
		return result_v;
	}

	static inline constexpr auto round_down(std::integral auto value_v, std::integral auto alignment_v = 0x1000u)
	{
		return value_v & ~(alignment_v - 1u);
	}

	static inline constexpr auto round_ceil(std::integral auto value_v, std::integral auto alignment_v = 0x1000u)
	{ 
		return round_down(value_v + alignment_v - 1u, alignment_v); 
	}

	template <typename T, std::size_t N>
	static inline constexpr auto make_filled_array(T default_v) -> std::array<T, N> {
		std::array<T, N> result_v{};
		result_v.fill(default_v);
		return result_v;
	}

	namespace detail
	{
		template <std::size_t Bytes, bool IsSigned>
		struct integer_by_size;

		template <> struct integer_by_size<1u, false> { using type = std::uint8_t;  };
		template <> struct integer_by_size<2u, false> { using type = std::uint16_t; };
		template <> struct integer_by_size<3u, false> { using type = std::uint32_t; };
		template <> struct integer_by_size<4u, false> { using type = std::uint32_t; };
		template <> struct integer_by_size<5u, false> { using type = std::uint64_t; };
		template <> struct integer_by_size<6u, false> { using type = std::uint64_t; };
		template <> struct integer_by_size<7u, false> { using type = std::uint64_t; };
		template <> struct integer_by_size<8u, false> { using type = std::uint64_t; };
		template <> struct integer_by_size<1u, true > { using type = std::int8_t;   };
		template <> struct integer_by_size<2u, true > { using type = std::int16_t;  };
		template <> struct integer_by_size<3u, true > { using type = std::int32_t;  };
		template <> struct integer_by_size<4u, true > { using type = std::int32_t;  };
		template <> struct integer_by_size<5u, true > { using type = std::int64_t;  };
		template <> struct integer_by_size<6u, true > { using type = std::int64_t;  };
		template <> struct integer_by_size<7u, true > { using type = std::int64_t;  };
		template <> struct integer_by_size<8u, true > { using type = std::int64_t;  };
	}

	template <std::integral Target, std::integral Source,
		std::size_t Ways = sizeof(Source) / sizeof(Target)>
	requires (sizeof(Target) <= sizeof(Source))
	static inline constexpr auto integral_split(Source value_v) -> 
		std::array<Target, Ways>
	{		
		if constexpr (Ways == 1u) {
			return 
			{ 
				static_cast<Target>(value_v >> (sizeof(Target) * 0u))
			};
		} else if constexpr (Ways == 2u) {
			return 
			{
				static_cast<Target>(value_v >> (sizeof(Target) * 0u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 8u)) 
			};
		}
		else if constexpr (Ways == 4u) 
		{
			return {
				static_cast<Target>(value_v >> (sizeof(Target) * 0u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 8u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 16u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 24u))
			};
		}
		else if constexpr (Ways == 8u) 
		{
			return {
				static_cast<Target>(value_v >> (sizeof(Target) * 0u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 8u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 16u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 24u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 32u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 40u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 48u)),
				static_cast<Target>(value_v >> (sizeof(Target) * 56u))
			};
		}
		else 
		{
			static_assert(sizeof(Target*)==0u, "Invalid split type");
		}
	}
		
	
	template<std::integral... Source, typename result_type = typename
		detail::integer_by_size<(sizeof(Source) + ...), (std::is_signed_v<Source> || ...)>::type>
	static inline constexpr auto integral_join(Source... value_v) -> result_type
	{
		result_type result_v{};
		(((result_v <<= sizeof(Source) * 8u) |= value_v), ...);
		return result_v;
	}

}