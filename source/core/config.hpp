#pragma once

#include <filesystem>
#include <functional>
#include <cstdint>
#include <cstddef>
#include <vector>

#include <utils/literals.hpp>

namespace core
{
	struct Machine;
	using namespace size_literals;

	struct Config 
	{
		auto SetMemorySize(std::size_t memorys_size_bytes_v) -> void;
		auto SetBootROM(std::uint64_t base_v, std::filesystem::path const& path_v) -> void;
		auto AddOptionROM(std::uint64_t base_v, std::filesystem::path const& path_v) -> void;
		auto AddProcessor(std::uint32_t processor_v) -> void;

	protected:
		friend struct Machine;
		void ApplyBeforeSetup(Machine& machine_v) const;
		void ApplyAfterSetup(Machine& machine_v) const;
	private:
		std::vector<std::uint32_t> m_Processors;
		std::size_t m_MemorySize { 640_KiB };
		std::pair<std::uint64_t, std::filesystem::path> m_BootROM { 0xF0000u, "BIOS.BIN" };
		std::vector<std::pair<std::uint64_t, std::filesystem::path>> m_OptionROMs;
	};

}