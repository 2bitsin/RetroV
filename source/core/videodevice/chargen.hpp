#pragma once

#include <core/videodevice/ramdac.hpp>
#include <utils/span.hpp>

#include <cstdint>
#include <cstddef>
#include <span>

namespace core 
{
	struct VideoDevice;
}

namespace core::videodevice
{
	using core::VideoDevice;

	struct CharGen
	{
		using output_type = RamDAC::output_type;

		CharGen(VideoDevice& device_v);

		auto DrawDot() -> output_type;
		auto NextRow() -> std::int32_t;
		auto LoadRow(uint32_t address_v, uint8_t last_v = 255u) -> std::int32_t;
		auto Reset() -> void;

		VideoDevice& m_Device;

#pragma pack(push, 1)
		// 0
	  std::uint64_t m_CharRows:5; // 0-31
		std::uint64_t m_CharCols:1; // 0=9 or 1=8 dots
		std::uint64_t m_ColorMode:1; // 0=gray or 1=color		
		std::uint64_t m_BlinkPhase:1; // 0=off or 1=on
		// 8
		std::uint64_t m_TableAddr:6;
		std::uint64_t m_DivideBy2:1;
		// 15
		std::uint64_t m_CharY:5;
		std::uint64_t m_CharX:4;
		// 24
		std::uint64_t m_ColIdx:8;
		// 32

#pragma pack(pop)
		std::uint32_t m_RowAddress;
		std::uint16_t m_RowCache[0x100];
	};

	static_assert(sizeof(CharGen)==8u);
}