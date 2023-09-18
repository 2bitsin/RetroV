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

		auto Refresh(double time_v) -> void;


	private:
		using BsCommon::m_Machine;
		using BsCommon::m_Device;

		uint16_t m_Cols;
		uint16_t m_Rows;
		uint16_t m_Mode;
		bool m_IsColor;
	};
}