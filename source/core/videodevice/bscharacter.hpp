#pragma once

#include <core/videodevice/bscommon.hpp>

namespace core::videodevice
{
	using core::Machine;
	using core::VideoDevice;


	struct BsCharacter: BsCommon
	{
		BsCharacter(core::Machine& machine_v, core::VideoDevice& device_v, 
			uint16_t hsize_v, uint16_t vsize_v, video_mode mode_v, uint16_t flags_v=0u);
		~BsCharacter() = default;

		auto Refresh(duration_type time_v) -> void;

	private:
		using BsCommon::m_Machine;
		using BsCommon::m_Device;

		video_mode m_Mode;
		uint16_t m_Cols;
		uint16_t m_Rows;
		bool m_IsColor;
	};
}