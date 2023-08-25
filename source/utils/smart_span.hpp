#pragma once

#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <span> 

namespace utils 
{

	namespace detail
	{
		template <typename T>
		struct default_span_deleter
		{			
			inline void operator()(std::span<T>& span_v) const noexcept
			{				
				if (span_v.data() != nullptr) {
					assert(span_v.size() > 0);
					delete[] span_v.data();
					span_v = std::span<T>{};
				}
			}
		};
	}

	using detail::default_span_deleter;

	template <typename T, typename Deleter = detail::default_span_deleter<T>>
	struct unique_span
		: public ::std::span<T> 
		, protected Deleter
	{
		
		unique_span() noexcept = default;

		unique_span(T* data_v, size_t size_v, Deleter deleter_v = Deleter{}) noexcept
			: ::std::span<T>{ data_v, size_v }
			, Deleter{ std::move(deleter_v) }
		{}

		unique_span(unique_span&& other_v) noexcept = default;
		unique_span(unique_span const& other_v) noexcept = delete;
		auto operator=(unique_span&& other_v) noexcept -> unique_span& = default;
		auto operator=(unique_span const& other_v) noexcept -> unique_span& = delete;

		~unique_span() noexcept
		{
			Deleter::operator()(*this);
		}
	private:		
	};

	static_assert(sizeof(unique_span<int>) == sizeof(::std::span<int>));

}