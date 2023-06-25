#pragma once

#include <iostream>
#include <filesystem>

#include <core/machine.hpp>
#include <devices/porte9hack.hpp>

struct HvDosMachine
{
	HvDosMachine(int argc, char** argv);
	auto Run() -> int;

	std::optional<core::Machine> machine_v;
	std::optional<core::PortE9HackDevice> pe9h_device_v;
};
