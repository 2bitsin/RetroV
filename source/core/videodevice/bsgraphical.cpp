#include "bsgraphical.hpp"
using core::videodevice::BsGraphical;

BsGraphical::BsGraphical(Machine& machine_v, VideoDevice& device_v, uint16_t hsize_v, uint16_t vsize_v, video_mode mode_v, uint16_t flags_v)
	: BsCommon(machine_v, device_v)
{}

auto BsGraphical::Refresh(duration_type) -> void
{

}
