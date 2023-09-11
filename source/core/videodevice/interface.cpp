#include <core/videodevice/interface.hpp>

#include <core/machine.hpp>
#include <core/processor.hpp>

#include <utils/literals.hpp>
#include <utils/surface.hpp>

using core::videodevice::Interface;

Interface::Interface(VideoDevice& host_v, Machine& machine_v)
	: m_Machine(machine_v)
	, m_VidHost(host_v)
{}
