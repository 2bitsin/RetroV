#pragma once

#include <core/videodevice/bscommon.hpp>

namespace core::videodevice
{
	using core::Machine;
	using core::VideoDevice;

	struct BsGraphical: BsCommon
	{
		BsGraphical(Machine& machine_v, VideoDevice& device_v, uint16_t hsize_v, uint16_t vsize_v, video_mode mode_v, uint16_t flags_v=0u);
		~BsGraphical() = default;

		auto Refresh(duration_type) -> void;
	};

}