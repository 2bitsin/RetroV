#pragma once

#include <core/hypervisor.hpp>

#include <cstdint>
#include <cstddef>
#include <any>

namespace device
{
	struct Interface
	{
		virtual ~Interface() = default;
		
		Interface(Interface const&) = delete;
		Interface(Interface&&) = delete;
		auto operator=(Interface const&) -> Interface& = delete;
		auto operator=(Interface&&) -> Interface& = delete;
		
		virtual auto Emulate() -> void = 0;		
	};

	static auto CreateDevice(std::string_view device_name_v, 
		std::any device_config_v) -> std::unique_ptr<Interface>;
}