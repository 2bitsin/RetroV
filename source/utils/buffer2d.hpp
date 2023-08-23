#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <span>

namespace utils
{

	template<typename T>
	struct buffer2d 	
	{		
		using value_type = T;

		buffer2d (std::uint32_t width_v, std::uint32_t height_v)
			: m_hsize	{ width_v  }
			, m_vsize	{ height_v }
			, m_data	{ std::make_unique<T[]>(m_hsize*m_vsize) }			
		{}

		auto width  () const noexcept -> std::uint32_t { return m_hsize; }
		auto height () const noexcept -> std::uint32_t { return m_vsize; }

		auto data() noexcept -> std::span<value_type> { return { m_data.get(), m_hsize*m_vsize }; }

		auto operator [] (std::uint32_t vindex_v) noexcept 
			-> std::span<value_type> 
		{ return { &m_data[vindex_v*m_hsize], m_hsize }; }

		auto operator [] (std::uint32_t vindex_v) const noexcept 
			-> std::span<value_type const> 
		{ return { &m_data[vindex_v*m_hsize], m_hsize }; }

		auto operator [] (std::uint32_t hindex_v, std::uint32_t vindex_v) noexcept 
			-> value_type& 
		{ return m_data[vindex_v*m_hsize + hindex_v]; }

		auto operator [] (std::uint32_t hindex_v, std::uint32_t vindex_v) const noexcept 
			-> value_type const& 
		{ return m_data[vindex_v*m_hsize + hindex_v]; }

	private:
		std::uint16_t m_hsize;
		std::uint16_t m_vsize;
		std::unique_ptr<value_type[]> m_bits;
	};

}