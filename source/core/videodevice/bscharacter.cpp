#include <core/videodevice/bscharacter.hpp>

using core::videodevice::BsCharacter;

BsCharacter::BsCharacter(core::Machine& machine_v, core::VideoDevice& device_v, 
	uint16_t horizontal_v, uint16_t vertical_v, uint16_t mode_v, uint16_t flags_v)
	: BsCommon(machine_v, device_v)
{
}