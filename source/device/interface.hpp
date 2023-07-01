#pragma once

#include <core/hypervisor_fwd.hpp>

#include <string_view>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <any>

namespace device
{
	struct Interface
	{
		Interface() = default;
		virtual ~Interface() = default;
		
		Interface(Interface const&) = delete;
		Interface(Interface&&) = delete;
		auto operator=(Interface const&) -> Interface& = delete;
		auto operator=(Interface&&) -> Interface& = delete;
		
		virtual auto Emulate() -> void = 0;		
	};

	static auto CreateDevice(core::Hypervisor&, std::string_view device_name_v, 
		std::any device_config_v) -> std::unique_ptr<Interface>;
}