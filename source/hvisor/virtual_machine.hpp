#pragma once

#include <hvisor/virtual_memory.hpp>
#include <hvisor/hypervisor.hpp>

#include <cstdint>
#include <cstddef>

#include <vector>

struct VirtualMachine
{
	struct config {
		std::size_t memory_size;
		std::filesystem::path path_to_bios;
	};

	VirtualMachine (config const&);
	~VirtualMachine ();

	auto Restart() -> void;
	auto Run() -> void;

protected:
	auto HandleIO(WHV_X64_IO_PORT_ACCESS_CONTEXT const& exit_v) -> void;

private:
	std::vector<VirtualMemory> m_memory;
	Hypervisor m_hypervisor;
};