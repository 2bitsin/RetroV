#include <device/interface.hpp>
#include <device/porte9.hpp>
#include <device/genericvga.hpp>

#include <functional>
#include <unordered_map>

using device::Interface;
using device::InterfacePtr;
using device::Config;

using core::Hypervisor;

auto CreateDevice(Hypervisor& hypervisor_v, std::string_view device_name_v, Config const& device_config_v)
	-> InterfacePtr 
{
	using factory_function = std::function<InterfacePtr(Hypervisor&, Config const&)>;

	static std::unordered_map<std::string_view, factory_function> const constructor_s {
		{"porte9", [](auto& hypervisor_v, auto& device_config_v) -> InterfacePtr {
			return std::make_unique<PortE9>(hypervisor_v, device_config_v);
		}},
		{ "genericvga", [](auto& hypervisor_v, auto& device_config_v) -> InterfacePtr {
			return std::make_unique<GenericVGA>(hypervisor_v, device_config_v);
		}}
	};

	auto const result_v = constructor_s.find(device_name_v);
	if (constructor_s.end()==result_v) return nullptr;	
	auto&& [_, factory_v] = *result_v;
	return factory_v(hypervisor_v, device_config_v);
}

auto Interface::GetCategory() const noexcept -> DeviceCatory {
  return DeviceCatory::kGeneric;
}
