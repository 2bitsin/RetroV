#include <device/interface.hpp>

auto device::CreateDevice(core::Hypervisor& hypervisor_v, std::string_view device_name_v, std::any device_config_v) 
-> std::unique_ptr<Interface> 
{
	if (device_name_v == "porte9") {
		return std::make_unique<PortE9>(hypervisor_v);
	}
	return nullptr;
}