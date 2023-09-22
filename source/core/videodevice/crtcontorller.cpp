#include <core/videodevice/crtcontroller.hpp>
#include <win32/windows.hpp>

#include <iterator>

using core::videodevice::CrtController;

CrtController::CrtController() { Reset(); }

auto CrtController::IoPortWrite(uint16_t port, uint8_t value) -> int32_t
{
	if (port == 0) { 
		m_Index = value & 0x1F; 
		return ERROR_SUCCESS; 
	}
	if (port == 1) {
		if (m_Index < std::size(m_Registers)) 
		{	m_Registers[m_Index] = value; 
			return ERROR_SUCCESS; }
		return ERROR_ACCESS_DENIED;
	}
	return ERROR_ACCESS_DENIED;
}

auto CrtController::IoPortFetch(uint16_t port) 
	-> std::tuple<int32_t, uint8_t> 
{
	if (port == 0) {
		return { ERROR_SUCCESS, m_Index }; 
	}
	if (port == 1) {
		if (m_Index < std::size(m_Registers)) 
			return { ERROR_SUCCESS, m_Registers[m_Index] }; 		
		return { ERROR_ACCESS_DENIED, 0 }; 
	}
	return { ERROR_ACCESS_DENIED, 0 };
}

auto CrtController::Reset() -> void
{
	m_HorzontalTotal = 0;
	m_EndHorzontalDisplay = 0;
	m_StartHorizontalBlanking = 0;
	m_EndHorizontalBlanking = 0;
	m_StartHorizontalRetrace = 0;
	m_EndHorizontalRetrace = 0;
	m_VerticalTotal = 0;
	m_Overflow = 0;
	m_PresetRowScan = 0;
	m_MaximumScanLine = 0;
	m_CursorStart = 0;
	m_CursorEnd = 0;
	m_StartAddressHi = 0;
	m_StartAddressLo = 0;
	m_CursorAddressHi = 0;
	m_CursorAddressLo = 0;
	m_VerticalRetraceStart = 0;
	m_VerticalRetraceEnd = 0;
	m_VerticalDisplayEnd = 0;
	m_Offset = 0;
	m_UnderlineLocation = 0;
	m_StartVerticalBlanking = 0;
	m_EndVerticalBlanking = 0;
	m_CrtModeControl = 0;
	m_LineCompare = 0;
}
