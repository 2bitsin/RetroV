#pragma once

#include <cassert>
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

		buffer2d()
			: m_hsize { 0 }
			, m_vsize { 0 }
			, m_data  { nullptr }
		{}

		buffer2d (std::uint16_t width_v, std::uint16_t height_v)
			: m_hsize	{ width_v  }
			, m_vsize	{ height_v }
			, m_data	{ std::make_unique<T[]>(m_hsize*m_vsize) }			
		{}

		buffer2d (buffer2d const& from_v) 
			: buffer2d { from_v.width(), from_v.height() }
		{
			auto const from_s = from_v.data();
			auto const dest_s = data();
			std::copy(from_s.begin(), from_s.end(), dest_s.begin());
		}

		buffer2d (buffer2d&& from_v) noexcept
			: m_hsize	{ std::exchange(from_v.m_hsize, 0) }
			, m_vsize	{ std::exchange(from_v.m_vsize, 0) }
			, m_data	{ std::exchange(from_v.m_data, nullptr) }
		{}

		auto swap(buffer2d& with_v) noexcept -> void
		{
			std::swap(m_hsize, with_v.m_hsize);
			std::swap(m_vsize, with_v.m_vsize);
			std::swap(m_data, with_v.m_data);
		}

		auto operator = (buffer2d const& from_v) -> buffer2d& {
			if (this != &from_v) {
				auto temp_v{ from_v };
				temp_v.swap(*this); }
			return *this;
		}

		auto operator = (buffer2d&& from_v) noexcept -> buffer2d& {
			if (this != &from_v) {
				auto temp_v{ std::move(from_v) };
				temp_v.swap(*this); }
			return *this;
		}

		auto width() const noexcept -> std::uint32_t { return m_hsize; }
		auto height() const noexcept -> std::uint32_t { return m_vsize; }

		auto data() noexcept -> std::span<value_type> { return { m_data.get(), (m_hsize*1u)*m_vsize }; }
		auto data() const noexcept -> std::span<value_type const> { return { m_data.get(), (m_hsize*1u)*m_vsize }; }

		auto operator [] (std::uint32_t vindex_v) noexcept 
			-> std::span<value_type> 
		{ 
			assert(vindex_v < m_vsize);
			return { &m_data[vindex_v*m_hsize], m_hsize }; 
		}

		auto operator [] (std::uint32_t vindex_v) const noexcept 
			-> std::span<value_type const> 
		{ 
			assert(vindex_v < m_vsize);
			return { &m_data[vindex_v*m_hsize], m_hsize }; 
		}

		auto operator [] (std::tuple<std::uint16_t, std::uint16_t> coords_v) noexcept -> value_type& 
		{
			auto const [hindex_v, vindex_v] = coords_v;
			assert(hindex_v < m_hsize);
			assert(vindex_v < m_vsize);
			return m_data[vindex_v*m_hsize + hindex_v]; 
		}
		
		auto operator [] (std::tuple<std::uint16_t, std::uint16_t> coords_v) const noexcept -> value_type const& 
		{ 
			auto const [hindex_v, vindex_v] = coords_v;
			assert(hindex_v < m_hsize);
			assert(vindex_v < m_vsize);
			return m_data[vindex_v*m_hsize + hindex_v]; 			
		}

		static auto filled(value_type solid_v, std::uint16_t width_v, std::uint16_t height_v) -> buffer2d
		{
			buffer2d result_v{ width_v, height_v };
			for (auto& pixel_v : result_v.data())
				pixel_v = solid_v;
			return result_v;
		}

	private:
		std::uint16_t m_hsize;
		std::uint16_t m_vsize;
		std::unique_ptr<value_type[]> m_data;
	};

}