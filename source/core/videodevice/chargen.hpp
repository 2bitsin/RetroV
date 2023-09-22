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

		auto NextFrame() -> std::int32_t;
		auto NextLine() -> std::int32_t;
		auto NextDot() -> output_type;
		
		auto Reset() -> void;

	protected:
		auto LoadRow(uint32_t address_v, uint8_t lastoff_v) -> std::int32_t;
	private:
		VideoDevice& m_Device;
	public:

		std::uint16_t m_RowCache[0x100];
	};

}