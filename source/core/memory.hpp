#pragma once

#include <win32/windows.hpp>

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <span>

struct Memory
{
	static inline constexpr const auto kPageSize = 4096;

	Memory (std::size_t size_v);
	Memory (std::size_t size_v, std::span<std::byte const> data_v, bool repeat_v = false);
	Memory (std::size_t size_v, std::filesystem::path const& path_v, bool repeat_v = false, 
		std::uint64_t offset = 0u, std::size_t length = 0u);
	~Memory ();

	auto operator= (Memory const&) -> Memory& = delete;
	auto operator= (Memory &&) noexcept -> Memory&;
	Memory(Memory const&) = delete;
	Memory(Memory &&) noexcept;

	auto Data () const noexcept -> std::byte const*;
	auto Data () noexcept -> std::byte*;
	auto Size () const noexcept -> std::size_t;

	auto View () const noexcept -> std::span<std::byte const>;
	auto View () noexcept -> std::span<std::byte>;

	auto Swap (Memory& other_v) noexcept -> void;

private:
	std::byte* m_Data { nullptr };
	std::size_t m_Size { 0u };
};