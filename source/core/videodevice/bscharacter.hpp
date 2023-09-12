#pragma once

#include <core/videodevice/bscommon.hpp>

namespace core::videodevice
{
	struct BsCharacter: BsCommon
	{
		BsCharacter(core::Machine& machine_v, core::VideoDevice& device_v);
		~BsCharacter();
	};
}
