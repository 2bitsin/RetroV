#pragma once

#include <cstdint>
#include <cstddef>

#include <tuple>

namespace core::videodevice
{
	struct CrtCtrl
	{
		CrtCtrl();
		auto IoPortWrite(uint16_t port, uint8_t value) -> int32_t;
		auto IoPortFetch(uint16_t port) -> std::tuple<int32_t, uint8_t>;
		auto Reset() -> void;

#pragma pack(push, 1)
		uint8_t m_Index;
		union
		{
			uint8_t m_Registers[0x19];
			struct
			{
				uint8_t m_HorzontalTotal;
				uint8_t m_EndHorzontalDisplay;
				uint8_t m_StartHorizontalBlanking;
				uint8_t m_EndHorizontalBlanking;
				uint8_t m_StartHorizontalRetrace;
				uint8_t m_EndHorizontalRetrace;
				uint8_t m_VerticalTotal;
				uint8_t m_Overflow;
				uint8_t m_PresetRowScan;
				uint8_t m_MaximumScanLine;
				uint8_t m_CursorStart;
				uint8_t m_CursorEnd;
				uint8_t m_StartAddressHi;
				uint8_t m_StartAddressLo;
				uint8_t m_CursorAddressHi;
				uint8_t m_CursorAddressLo;
				uint8_t m_VerticalRetraceStart;
				uint8_t m_VerticalRetraceEnd;
				uint8_t m_VerticalDisplayEnd;
				uint8_t m_Offset;
				uint8_t m_UnderlineLocation;
				uint8_t m_StartVerticalBlanking;
				uint8_t m_EndVerticalBlanking;
				uint8_t m_CrtModeControl;
				uint8_t m_LineCompare;
			};
		};
#pragma pack(pop)
	};
}