#include <hvisor/hypervisor.hpp>
#include <hvisor/virtual_machine.hpp>
#include <hvisor/virtual_memory.hpp>

#include <stdexcept>

VirtualMachine::VirtualMachine(config const& config_v)
{
	static constexpr auto ram_permissions = WHV_MAP_GPA_RANGE_FLAGS( WHvMemoryAccessExecute | WHvMemoryAccessWrite | WHvMemoryAccessRead);
	static constexpr auto rom_permissions = WHV_MAP_GPA_RANGE_FLAGS( WHvMemoryAccessExecute | WHvMemoryAccessRead);
	static constexpr auto dev_permissions = WHV_MAP_GPA_RANGE_FLAGS( WHvMemoryAccessWrite | WHvMemoryAccessRead);

	auto memory_size_v = config_v.memory_size;

	if (memory_size_v < 1024u) {
		throw std::runtime_error ("Please specify more then 1KB of memory.");
	}

	auto conventional_memory_size_v = std::min<std::size_t>(640u*1024u, memory_size_v);
	memory_size_v -= conventional_memory_size_v;

	// 0x00000 - 0x9FFFF : Conventional Memory
	m_memory.emplace_back(conventional_memory_size_v);
	m_hypervisor.MapPhysical(m_memory.back(), 0x00000u, ram_permissions);

	// 0xA0000 - 0xAFFFF : VGA Bitmap Memory
	m_memory.emplace_back(0x10000u);
	m_hypervisor.MapPhysical(m_memory.back(), 0xA0000u, dev_permissions);

	// 0xB8000 - 0xBFFFF : VGA Text Memory
	m_memory.emplace_back(0x8000u);
	m_hypervisor.MapPhysical(m_memory.back(), 0xB8000u, dev_permissions);

	// 0xF0000 - 0xFFFFF : BIOS ROM
	m_memory.emplace_back(config_v.path_to_bios, 0x10000u);
	m_hypervisor.MapPhysical(m_memory.back(), 0xF0000u, rom_permissions);

	// 0x100000 - 0x100000 + memory_size_v : Extended Memory
	m_memory.emplace_back(memory_size_v);
	m_hypervisor.MapPhysical(m_memory.back(), 0x100000u, ram_permissions);
}

VirtualMachine::~VirtualMachine() 
{

}