#pragma once

#include <core/hypervisor.hpp>

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct BiosDisk
	{
		static inline constexpr auto kSectorSize = 512u;
		struct Geometry
		{				
			std::uint64_t SectorsLBA;
			std::uint32_t Tracks;
			std::uint16_t Heads;
			std::uint16_t Sectors;
		};

		union Index 
		{
			struct
			{
				std::uint32_t Track;
				std::uint16_t Head;
				std::uint16_t Sector;
			};
			std::uint64_t SectorLBA;
		};

		BiosDisk(Hypervisor& hypervisor_v, std::uint8_t drive_id_v);
		~BiosDisk();

		BiosDisk(BiosDisk&&) = delete;
		BiosDisk(BiosDisk const&) = delete;
		auto operator = (BiosDisk&&) -> BiosDisk& = delete;
		auto operator = (BiosDisk const&) -> BiosDisk& = delete;

		auto MountImage(std::filesystem::path const& path_v, Geometry const& geometry_v, bool use_chs_v = true) -> void;
		auto MountImage(std::filesystem::path const& path_v, bool use_chs_v = false) -> void;
		auto Fetch(std::span<std::byte>& buffer_v, Index const& index_v, bool uselba_v = true) -> std::size_t;
		auto Write(std::span<std::byte const>& buffer_v, Index const& index_v, bool uselba_v = true) -> std::size_t;
		auto Unmount() -> void;

		auto Int13h(core::Hypervisor& hypervisor_v, core::RegisterFile& registers_v, std::uint32_t cpuindex_v) -> bool;
		auto Int19h(core::Hypervisor& hypervisor_v, core::RegisterFile& registers_v, std::uint32_t cpuindex_v) -> bool;

	private:
		core::Hypervisor& m_Hypervisor;
		std::uint8_t m_DriveID { 0u };
		std::uint32_t m_Int13h { 0u };
		std::uint32_t m_Int19h { 0u };
		bool m_UseLBA { true };
		Geometry m_Geometry { 0, 0, 0, 0 };
		std::fstream m_File;
		std::uint8_t m_LastError { 0u };
	};
}