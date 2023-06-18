#pragma once

#include <cstdint>
#include <cstddef>

#include <filesystem>
#include <limits>

#include <span>

struct Hypervisor;

struct VirtualMemory
{
	VirtualMemory();
	VirtualMemory(std::size_t size_v);
	VirtualMemory(std::filesystem::path const& path_v, std::size_t size_limit_v = 
		std::numeric_limits<std::size_t>::max());
	~VirtualMemory();


	operator std::span<std::byte>() const { return { m_data, m_size }; }
	inline auto data() const -> std::byte* { return m_data; };
	inline auto size() const -> std::size_t { return m_size; };

	VirtualMemory(VirtualMemory const&) = delete;
	auto operator = (VirtualMemory const&)->VirtualMemory& = delete;

	VirtualMemory(VirtualMemory &&) noexcept ;
	auto operator = (VirtualMemory &&) noexcept ->  VirtualMemory&;

	auto swap(VirtualMemory&)->void;

private:
	std::size_t m_size;
	std::byte* m_data;
};