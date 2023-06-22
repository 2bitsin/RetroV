#pragma once

#include <filesystem>
#include <functional>
#include <cstdint>
#include <cstddef>
#include <vector>

struct Machine;

struct Config 
{
	void SetMemorySize(std::size_t memorys_size_bytes_v);
	void SetBootROM(std::uint64_t base_v, std::filesystem::path const& path_v);
	void AddOptionROM(std::uint64_t base_v, std::filesystem::path const& path_v);
	
protected:
	friend struct Machine;
	void ApplyBeforeSetup(Machine& machine_v) const;
	void ApplyAfterSetup(Machine& machine_v) const;
private:
	std::size_t m_MemorySize { 0 };
	std::pair<std::uint64_t, std::filesystem::path> m_BootROM;
	std::vector<std::pair<std::uint64_t, std::filesystem::path>> m_OptionROMs;
};
