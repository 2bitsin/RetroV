#pragma once

#include <core/videodevice/bscommon.hpp>

namespace core::videodevice
{
	using core::Machine;
	using core::VideoDevice;

	struct BsGraphical: BsCommon
	{
		BsGraphical(Machine& machine_v, VideoDevice& device_v);
		~BsGraphical();
	};

}