#pragma once

#include <core/videodevice/bscommon.hpp>

namespace core::videodevice
{
	using core::Machine;
	using core::VideoDevice;


	struct BsCharacter: BsCommon
	{
		BsCharacter(core::Machine& machine_v, core::VideoDevice& device_v, 
			uint16_t horizontal_v, uint16_t vertical_v, uint16_t mode_v, uint16_t flags_v);
		~BsCharacter() = default;
	private:
		
	};
}