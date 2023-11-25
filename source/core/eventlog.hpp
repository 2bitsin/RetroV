#pragma once

#include <string_view>
#include <filesystem>
#include <string>
#include <format>

#include <cstdint>
#include <cstddef>

#include <core/accessflags.hpp>
#include <utils/limited_span.hpp>

namespace core
{
	struct EventLog
	{
		auto StartMachine() const -> void;
		auto StopMachine() const -> void;
		auto ResetMachine() const -> void;

		auto MapGpaRange(void* addr_v, uint64_t base_v, uint64_t size_v, Access access_v) const -> void; 
		auto MapGpaRangeFromFile(void* addr_v, uint64_t base_v, uint64_t size_v, std::filesystem::path const& path_v, uint64_t offset_v, uint64_t length_v) const -> void;
		auto UnmapGpaRange(uint64_t base_v, uint64_t size_v) const -> void;
	
		auto UnrealModeEnabled(uint32_t) const -> void;
    auto UnrealModeDisabled(uint32_t) const -> void;

		auto EmitPostCode(utils::limited_span<std::byte, 4u> data_v) const -> void;

		auto VCpuExited(uint32_t vcpu_index_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_context_v) const -> void;

		auto IRQState(uint32_t vcpu_index_v, uint16_t irq_v) const -> void;

		auto DebugTrap(uint32_t vcpu_index_v, uint64_t linaddr_v, uint16_t segsel_v, uint64_t offset_v) const -> void;

		auto UnhandledException(uint32_t vcpu_index_v, const WHV_VP_EXCEPTION_CONTEXT& exception_v, const WHV_VP_EXIT_CONTEXT& context_v) const -> void;
		auto UnhandledMSR(uint32_t vcpu_index_v, WHV_VP_EXIT_CONTEXT const& context_v, WHV_X64_MSR_ACCESS_CONTEXT const& access_v) const -> void;

		auto EmulatorFailed(WHV_RUN_VP_EXIT_CONTEXT const& context_v, int32_t status_v, WHV_EMULATOR_STATUS emulator_status_v) const -> void;
		
		EventLog(std::string_view module_v);

	private:
		std::string m_Module;
	};
}