#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/whvpartition.hpp>
#include <win32/memory.hpp>

#include <core/accessflags.hpp>


namespace core
{
	

	struct Memory
	{ 
		/****************************************
		 * 
		 *   base_v and size_v specified in pages
		 * 
		 ****************************************/
		 
		Memory (win32::WHvPartition const& partition_v, std::uint64_t base_v, std::uint64_t size_v, Access prot_v);

		Memory (Memory const&) = delete;
		auto operator = (Memory const&) = delete;

		Memory (Memory&&) noexcept;
		auto operator = (Memory&&) noexcept -> Memory&;

		auto swap (Memory& other_v) noexcept -> void;

		~Memory ();

		auto Base () const noexcept -> std::uint64_t;
		auto Size () const noexcept -> std::uint64_t;
		auto Data () const noexcept -> std::span<std::byte>;

		auto Load (std::filesystem::path src_path_v, std::uint64_t dst_offset_v=0u, std::uint64_t src_offset_v=0u, 
			std::uint64_t src_length_v=0xFFFFFFFFFFFFFFFFu) -> std::size_t;

		auto CopyDirtyPagesTo(std::span<std::byte> target_v) -> std::int32_t;

		auto Unmap () noexcept -> std::int32_t;
		auto Map () noexcept -> std::int32_t;

	private:
		win32::WHvPartition const* m_Partition;
		win32::unique_span<std::byte> m_Data;
		std::uint64_t m_Base;
		Access m_Flags;
		mutable std::vector<void const*> m_Dirty;
	};


}