#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/whvpartition.hpp>

#include <core/accessflags.hpp>


namespace core
{
	

	struct VirtualMemory
	{ 
		/****************************************
		 * 
		 *   base_v and size_v specified in pages
		 * 
		 ****************************************/
		 
		VirtualMemory (win32::WHvPartition const& partition_v, std::uint64_t base_v, std::uint64_t size_v, Access prot_v);
		~VirtualMemory ();

		auto Base () const noexcept -> std::uint64_t;
		auto Size () const noexcept -> std::uint64_t;
		auto Data () const noexcept -> std::byte*;

		auto Load (std::filesystem::path) -> std::size_t;

	private:
		win32::WHvPartition const& m_Partition;
		std::uint64_t m_Base;
		std::uint64_t m_Size;
		std::byte* m_Data;
	};


}