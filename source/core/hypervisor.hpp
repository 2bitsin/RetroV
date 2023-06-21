#pragma once

#include <filesystem>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

#include <core/machine_monitor.hpp>

namespace core
{

	struct hypervisor : public object
	{
		static auto create (std::unique_ptr<machine_monitor>) -> std::unique_ptr<hypervisor>;

		virtual auto map_physical_memory(void*, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void = 0;
		virtual auto unmap_physical_memory(std::uint64_t base_v, std::uint64_t size_v) -> void = 0;
		

		virtual auto start_machine() -> void = 0;
	};





}