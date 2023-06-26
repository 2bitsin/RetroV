#pragma once

#include <iostream>
#include <filesystem>

#include <core/machine.hpp>
#include <devices/porte9handler.hpp>

struct HvDosMachine
{
	HvDosMachine(int argc, char** argv);
	auto Run() -> int;

	std::optional<core::Machine> machine_v;
	std::optional<core::PortE9Handler> porte9_v;
};
