#include <device/interface.hpp>
#include <device/porte9.hpp>
#include <device/genericvga.hpp>

#include <functional>
#include <unordered_map>

auto device::CreateDevice(core::Hypervisor& hypervisor_v, std::string_view device_name_v, Config const& device_config_v)
	-> device::InterfacePtr 
{
	using factory_function = std::function<device::InterfacePtr(core::Hypervisor&, Config const&)>;

	static std::unordered_map<std::string_view, factory_function> const constructor_s {
		{"porte9", [](auto& hypervisor_v, auto& device_config_v) -> device::InterfacePtr {
			return std::make_unique<PortE9>(hypervisor_v, device_config_v);
		}},
		{ "genericvga", [](auto& hypervisor_v, auto& device_config_v) -> device::InterfacePtr {
			return std::make_unique<GenericVGA>(hypervisor_v, device_config_v);
		}}
	};

	auto const result_v = constructor_s.find(device_name_v);
	if (constructor_s.end()==result_v) return nullptr;	
	auto&& [_, factory_v] = *result_v;
	return factory_v(hypervisor_v, device_config_v);
}