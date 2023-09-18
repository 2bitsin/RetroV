#pragma once

#include <cstdint>
#include <cstddef>
#include <chrono>

#include <win32/memory.hpp>
#include <win32/chrono.hpp>

#include <utils/smart_span.hpp>
#include <utils/span.hpp>
#include <bios/com/hypercall.hpp>

namespace core
{
	struct Machine;
	struct VideoDevice;
}

namespace core::videodevice
{
	using duration_type = win32::filetime_clock::duration;
	using buffer_type = win32::unique_span<std::byte>;	

	enum video_mode : uint32_t
	{
		character_color_8x8  = hypercall::VIDEO_MODE_CHARACTER_COLOR_8X8,
		character_color_8x14 = hypercall::VIDEO_MODE_CHARACTER_COLOR_8X14,
		character_color_8x16 = hypercall::VIDEO_MODE_CHARACTER_COLOR_8X16,
		//= 0x03,
		character_color_9x8  = hypercall::VIDEO_MODE_CHARACTER_COLOR_9X8,
		character_color_9x14 = hypercall::VIDEO_MODE_CHARACTER_COLOR_9X14,
		character_color_9x16 = hypercall::VIDEO_MODE_CHARACTER_COLOR_9X16,
		//= 0x07,

		graphical_4bpp  = hypercall::VIDEO_MODE_GRAPHICAL_4BPP,
		graphical_8bpp  = hypercall::VIDEO_MODE_GRAPHICAL_8BPP,
		graphical_1bpp  = hypercall::VIDEO_MODE_GRAPHICAL_1BPP,
		graphical_2bpp  = hypercall::VIDEO_MODE_GRAPHICAL_2BPP,
		graphical_16bpp = hypercall::VIDEO_MODE_GRAPHICAL_16BPP,
		graphical_24bpp = hypercall::VIDEO_MODE_GRAPHICAL_24BPP,
		graphical_32bpp = hypercall::VIDEO_MODE_GRAPHICAL_32BPP,		
		//= 0x0F,

		character_mono_8x8  = hypercall::VIDEO_MODE_CHARACTER_MONO_8X8,
		character_mono_8x14 = hypercall::VIDEO_MODE_CHARACTER_MONO_8X14,
		character_mono_8x16 = hypercall::VIDEO_MODE_CHARACTER_MONO_8X16,		
		//= 0x13,
		character_mono_9x8  = hypercall::VIDEO_MODE_CHARACTER_MONO_9X8,
		character_mono_9x14 = hypercall::VIDEO_MODE_CHARACTER_MONO_9X14,
		character_mono_9x16 = hypercall::VIDEO_MODE_CHARACTER_MONO_9X16,
		//= 0x17,

	};

	struct BsCommon
	{
		BsCommon(Machine& machine_v, VideoDevice& device_v);
		~BsCommon() = default;

	protected:
		Machine& m_Machine;
		VideoDevice& m_Device;
	};
}