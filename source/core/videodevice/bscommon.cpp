#include "bscommon.hpp"

using core::videodevice::BsCommon

BsCommon::BsCommon(core::Machine& machine_v, core::VideoDevice& device_v)
	: m_Machine(machine_v)
	, m_Device(device_v)	
{
}
