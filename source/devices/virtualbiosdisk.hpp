#pragma once

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <span>

#include <core/vchandlerbridge.hpp>

namespace core
{
	struct VirtualBiosDisk
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

		VirtualBiosDisk(Machine& machine_v, std::uint8_t drive_id_v);
		~VirtualBiosDisk() = default;

		VirtualBiosDisk(VirtualBiosDisk&&) = delete;
		VirtualBiosDisk(VirtualBiosDisk const&) = delete;
		auto operator = (VirtualBiosDisk&&) -> VirtualBiosDisk& = delete;
		auto operator = (VirtualBiosDisk const&) -> VirtualBiosDisk& = delete;

		auto MountImage(std::filesystem::path const& path_v, Geometry const& geometry_v, bool use_chs_v = true) -> void;
		auto MountImage(std::filesystem::path const& path_v, bool use_chs_v = false) -> void;
		auto Fetch(std::span<std::byte>& buffer_v, Index const& index_v, bool uselba_v = true) -> std::size_t;
		auto Write(std::span<std::byte const>& buffer_v, Index const& index_v, bool uselba_v = true) -> std::size_t;
		auto Unmount() -> void;

		auto VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v, 
			std::uint16_t callno_v = VCHandler::kLastCall) -> bool;

		auto Int13h(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v) -> bool;
		auto Int19h(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v) -> bool;

	private:
		VCHandlerBridge<VirtualBiosDisk&> m_Bridge13h;
		VCHandlerBridge<VirtualBiosDisk&> m_Bridge19h;
		bool m_UseLBA { true };
		std::uint8_t m_DriveID { 0u };
		Geometry m_Geometry { 0, 0, 0, 0 };
		std::fstream m_File;
		std::uint8_t m_LastError { 0u };
	};
}