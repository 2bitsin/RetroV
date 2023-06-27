#pragma once

#include <iostream>
#include <filesystem>

#include <core/machine.hpp>
#include <devices/porte9handler.hpp>
#include <devices/virtualbiosdisk.hpp>

struct HvDosMachine
{
	HvDosMachine(int argc, char** argv);
	auto Run() -> int;

	template<typename... T>
	auto MountImage(T&&...args) -> void {
		return disk0_v->MountImage(std::forward<T>(args)...);
	}

	auto UnmountImage() -> void {
		return disk0_v->Unmount();
	}
private:
	std::optional<core::Machine> machine_v;
	std::optional<core::PortE9Handler> porte9_v;
	std::optional<core::VirtualBiosDisk> disk0_v;
};
