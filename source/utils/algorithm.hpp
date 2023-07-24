#pragma once

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

}