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

private:
	HyperVisor m_hypervisor;
	std::vector<VirtualMemory> m_memory;
};