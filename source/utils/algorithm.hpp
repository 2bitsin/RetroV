#pragma once

#include <type_traits>
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


	template <typename T> requires (std::is_integral_v<T>)
		static inline auto bitset_to_string(T bits) -> std::string
	{
		std::string result_v;
		for (auto i = 0u; i < 8u * sizeof(T); i += 1u) {
			if (bits & 1u) {
				if (!result_v.empty())
					result_v += ", ";
				result_v += std::to_string(i);
			}
			bits >>= 1u;
		}
		return result_v;
	}

}