#include <core/videodevice/chargen.hpp>
#include <core/videodevice.hpp>

using core::videodevice::CharGen;

CharGen::CharGen(VideoDevice& device_v)
: m_Device(device_v)
{
	Reset();
}

auto CharGen::LoadRow(uint32_t address_v, uint8_t lastoff_v) -> std::int32_t
{
	return ERROR_SUCCESS;
}

auto CharGen::Reset() -> void
{
}

auto CharGen::NextLine() -> std::int32_t
{
	return ERROR_SUCCESS;
}

auto CharGen::NextFrame() -> std::int32_t
{
	return ERROR_SUCCESS;
}


auto CharGen::NextDot() -> output_type
{
	return { 0 };
}
