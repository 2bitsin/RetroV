#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <span>

namespace utils 
{
	template <typename T>
	static inline auto take_span(std::span<T>& from_v, std::size_t size_v) -> std::span<T>
	{
		if (size_v < 1u || from_v.size() < 1u)
			return std::span<T>{};		
		size_v = std::min(size_v, from_v.size());
		auto shard_v = from_v.first(size_v);
		if (size_v < from_v.size()) {
			from_v = from_v.subspan(size_v);
		} else {
			from_v = std::span<T>{};
		}
		return shard_v;
	}

	template <typename T>
		requires (std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>)
	static inline auto as_bytes(T const& value) noexcept -> std::span<std::byte const> {
		return { reinterpret_cast<std::byte const*>(&value), sizeof(T) };
	}

	template <typename T>
		requires (std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>)
	static inline auto as_mutable_bytes(T& value) noexcept -> std::span<std::byte> {
		return { reinterpret_cast<std::byte*>(&value), sizeof(T) };
	}

	template <typename T, typename Q>
		requires (std::is_trivial_v<Q>&& std::is_trivial_v<T>)
	static inline auto mutable_span_as(std::span<Q> input_v) -> std::span<T> {
		return { reinterpret_cast<T*>(input_v.data()), input_v.size_bytes() / sizeof(T) };
	}

	template <typename T, typename Q>
		requires (std::is_trivial_v<Q>&& std::is_trivial_v<T>)
	static inline auto span_as(std::span<Q const> input_v) -> std::span<T const> {
		return { reinterpret_cast<T const*>(input_v.data()), input_v.size_bytes() / sizeof(T) };
	}

	template <typename T, auto Limit>	
	struct alignas(sizeof(void*)) limited_span 
	{
		using value_type = T;
		using size_type = decltype(Limit);
    static inline constexpr const size_type limit = Limit;

		constexpr inline limited_span(value_type* data_v, size_type size_v) noexcept
			: m_data{ data_v }
			, m_size{ std::min<size_type>(size_v, limit) }
		{}

		template <auto Size> requires (Size <= limit)
		constexpr inline limited_span(value_type (&data_v)[Size]) noexcept
			: m_data{ data_v }
			, m_size{ Size }
		{}

		constexpr inline limited_span(std::span<value_type> data_v) noexcept
			: m_data{ data_v.data() }
			, m_size{ std::min<size_type>(data_v.size(), limit) }
		{}

		template <auto Size> requires (Size <= limit && Size != std::dynamic_extent)
		constexpr inline limited_span(std::span<value_type, Size> data_v) noexcept
			: m_data{ data_v.data() }
			, m_size{ std::min<size_type>(data_v.size(), limit) }
		{}

		template <auto Size> requires (Size <= limit)
		constexpr inline limited_span(limited_span<value_type, Size> const& data_v) noexcept
			: m_data{ data_v.data() }
			, m_size{ std::min<size_type>(data_v.size(), limit) }
		{}

		constexpr inline auto size() const noexcept -> size_type { return m_size; }
		constexpr inline auto data() const noexcept -> value_type* { return m_data; }

		constexpr inline auto first(size_type size_v) const noexcept -> limited_span<value_type, limit> {
			return { m_data, std::min(size_v, m_size) };
		}

		constexpr inline auto subspan(size_type offset_v, size_type size_v) const noexcept
			-> limited_span<value_type, limit> 
		{
			if (offset_v >= m_size)
				return {};
			return { m_data + offset_v, std::min(size_v, m_size - offset_v) };		
		}

		constexpr inline auto operator [](size_type index_v) const noexcept -> value_type& {
			if (index_v >= m_size)
				throw std::out_of_range{ "index out of range" };			
			return m_data[index_v];
		}

		constexpr inline auto empty () const noexcept -> bool { return m_size == 0u; }

		constexpr inline operator std::span<value_type>() const noexcept {
			return { m_data, m_size };
		}

		constexpr inline operator std::span<value_type const>() const noexcept {
			return { m_data, m_size };
		}

		constexpr inline auto begin() const noexcept -> value_type* { return m_data; }
		constexpr inline auto end() const noexcept -> value_type* { return m_data + m_size; }
		constexpr inline auto cbegin() const noexcept -> value_type const* { return m_data; }
		constexpr inline auto cend() const noexcept -> value_type const* { return m_data + m_size; }

	
	private:
		value_type* m_data;
		size_type m_size;
	};


	template <typename T> requires (std::is_trivial_v<T>)
		static inline auto as_static_bytes(T const& value_v) 
		-> utils::limited_span<std::byte const, sizeof(T)> 
	{
		using type = std::byte const [sizeof(T)];
		return { reinterpret_cast<type&>(value_v) };
	}

	template <typename T> requires (std::is_trivial_v<T>)
		static inline auto as_static_mutable_bytes(T& value_v) 
		-> utils::limited_span<std::byte, sizeof(T)> 
	{
		using type = std::byte[sizeof(T)];
		return { reinterpret_cast<type&>(value_v) };
	}

}