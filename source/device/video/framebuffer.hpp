#pragma once

#include <cstdint>
#include <memory>
#include <span>

namespace device {
	struct framebuffer
	{
		constexpr framebuffer(std::uint32_t hsize, std::uint32_t vsize) {

		}
	  
		~framebuffer();

		
		std::uint32_t hsize() const { return m_hsize; }
		std::uint32_t vsize() const { return m_vsize; }

		auto data() const -> std::span<std::byte const> { 
			return { m_data.get(), m_vsize*(m_stride?m_stride:m_hsize) }; 
		}

		auto data() -> std::span<std::byte> { 
			return { m_data.get(), m_vsize*(m_stride?m_stride:m_hsize) }; 
		}
	
	private:
		std::unique_ptr<std::byte []> m_data;
		std::uint32_t m_stride;
		std::uint32_t m_hsize;
		std::uint32_t m_vsize;
	};

}
