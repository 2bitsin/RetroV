#pragma once

#include <type_traits>
#include <concepts>
#include <iterator>
#include <string>

namespace utils
{

  template<typename Iterator, typename T>
  constexpr Iterator upper_bound(Iterator first, Iterator last, const T& value)
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

	template<typename Iterator, typename T>
	constexpr Iterator lower_bound(Iterator first, Iterator last, const T& value)
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

	constexpr auto round_down(std::integral auto value_v, std::integral auto alignment_v = 0x1000u) 
	{
		return value_v & ~(alignment_v - 1u);
	}

	constexpr auto round_ceil(std::integral auto value_v, std::integral auto alignment_v = 0x1000u) 
	{ 
		return round_down(value_v + alignment_v - 1u, alignment_v); 
	}

}