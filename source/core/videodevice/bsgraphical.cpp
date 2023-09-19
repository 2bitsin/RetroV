#include <core/videodevice/bscommon.hpp>
#include <core/videodevice/bsgraphical.hpp>
#include <core/videodevice.hpp>
#include <core/machine.hpp>

#include <utils/surface.hpp>

using core::videodevice::BsGraphical;

BsGraphical::BsGraphical(Machine& machine_v, VideoDevice& device_v, uint16_t hsize_v, uint16_t vsize_v, video_mode mode_v, uint16_t flags_v)
	: BsCommon { machine_v, device_v }
	, m_HSize  { hsize_v }
	, m_VSize  { vsize_v }
{}

auto BsGraphical::Refresh(duration_type) -> void
{
	auto& display_v = m_Machine.GetDisplay();
	auto surface_v = display_v.AcquireSurface(m_HSize, m_VSize);
	{
		utils::surface_view<std::uint32_t> surface_view_v{ surface_v };
		for (auto&& what_v : surface_view_v) {
			what_v = 0xFF'00'00'FFu;
		}
	}
	display_v.Present(std::move(surface_v));
}
