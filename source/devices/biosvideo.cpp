#include <devices/biosvideo.hpp>
#include <iostream>

using core::BiosVideo;

BiosVideo::BiosVideo(Hypervisor& hypervisor_v)
	: m_Hypervisor(hypervisor_v)
	, m_Int10h(m_Hypervisor.GetVcManager().RegisterCallback(0x10u,
		[this](auto& hypervisor_v, auto& registers_v, auto cpuindex_v, auto callno_v) -> bool {
			return Int10h(hypervisor_v, registers_v, cpuindex_v);
		}))
{
}

BiosVideo::~BiosVideo()
{
	m_Hypervisor.GetVcManager().UnregisterCallback(0x10u, m_Int10h);
}

auto BiosVideo::Int10h(core::Hypervisor& hypervisor_v, core::RegisterFile& R, std::uint32_t cpuindex_v) -> bool
{
	switch (R.ah) {
	case 0x0eu: // Teletype output
		std::cout << static_cast<char>(R.al);
		break;

	default:
		__debugbreak();
		break;
	}
	return true;
}
