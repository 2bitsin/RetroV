#pragma once

#include <cstddef>
#include <cstdint>

#include <span>

namespace device::video 
{
	struct text_grid
	{
		inline text_grid(device::video::font& font_v, std::uint16_t cols_v, std::uint16_t rows_v)
			: m_font(font_v), m_cols(cols_v), m_rows(rows_v)
		{}

		auto draw(auto const& source_v, auto&& target_v) -> void 
		{
			auto const source_h = source_v.height()*m_font.rows;
			auto const source_w = source_v.width()*m_font.cols;
			auto const target_h = target_v.height();
			auto const target_w = target_v.width();

			for (auto sy = 0u; sy < source_v.height(); ++sy)
			for (auto sx = 0u; sx < source_v.width(); ++sx) 
			{
				auto const char_and_attr_v = source_v[sx, sy];
				auto const char_v = (char_and_attr_v >> 0u)&0xFFu;
				auto const attr_v = (char_and_attr_v >> 8u)&0xFFu;
			}

		}

	protected:
		inline auto height () const noexcept -> std::uint32_t { return m_rows * m_font.rows; }
		inline auto width  () const noexcept -> std::uint32_t { return m_cols * m_font.cols; }

		inline auto draw_row(auto&& target_v, std::uint8_t cols_v, std::uint32_t yoff_v, 
			std::uint32_t xoff_v, std::span<std::byte const> mask_v, auto&& attr_v) -> void 
		{
			for(auto curr_v = 0u; curr_v < cols_v; curr_v += 1u) {
				auto const index_v = (mask_v[curr_v / 8u] >> (curr_v % 8u)) & 1u;
				target_v[xoff_v + curr_v, yoff_v] = attr_v[index_v];
			}
		}

	private:
		video::font&  m_font;
		std::uint16_t m_cols;
		std::uint16_t m_rows;
	
	};
}