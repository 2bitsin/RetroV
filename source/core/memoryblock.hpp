#pragma once

#include <win32/windows.hpp>

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct MemoryBlock
	{
		static inline constexpr const auto kPageSize = 4096;
	
		MemoryBlock ();
		MemoryBlock (std::size_t size_v);
		MemoryBlock (std::span<std::byte const> data_v, std::size_t size_v = 0u, bool repeat_v = false);
		MemoryBlock (std::filesystem::path const& path_v, std::size_t size_v = 0u, bool repeat_v = false,
			std::uint64_t offset = 0u, std::size_t length = 0u);
		~MemoryBlock ();
	
		auto operator= (MemoryBlock const&) -> MemoryBlock& = delete;
		auto operator= (MemoryBlock &&) noexcept -> MemoryBlock&;
		MemoryBlock(MemoryBlock const&) = delete;
		MemoryBlock(MemoryBlock &&) noexcept;
	
		auto Data () const noexcept -> std::byte const*;
		auto Data () noexcept -> std::byte*;
		auto Size () const noexcept -> std::size_t;
	
		auto View () const noexcept -> std::span<std::byte const>;
		auto View () noexcept -> std::span<std::byte>;
	
		auto Swap (MemoryBlock& other_v) noexcept -> void;

		auto Rellocate (std::size_t size_v) -> void;
		auto Release () noexcept -> void;
	
	private:
		std::byte* m_Data { nullptr };
		std::size_t m_Size { 0u };
	};
}