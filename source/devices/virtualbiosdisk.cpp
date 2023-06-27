#include "virtualbiosdisk.hpp"

#include <format>
#include <system_error>

using core::VirtualBiosDisk;

enum DiskStatusCode : std::uint8_t
{
	SUCCESSFUL_COMPLETION = 0x00u,     //successful completion
	INVALID_FUNCTION_OR_PARAMETER = 0x01u,     //invalid function in AH or invalid parameter
	ADDRESS_MARK_NOT_FOUND = 0x02u,     //address mark not found
	DISK_WRITE_PROTECTED = 0x03u,     //disk write - protected
	SECTOR_NOT_FOUND_OR_READ_ERROR = 0x04u,     //sector not found / read error
	RESET_FAILED_HD = 0x05u,     //reset failed(hard disk)
	DATA_NOT_VERIFIED_TI_PRO_PC = 0x05u,    //data did not verify correctly(TI Professional PC)
	DISK_CHANGED_FLOPPY = 0x06u,     //disk changed(floppy)
	DRIVE_PARAM_ACTIVITY_FAILED_HD = 0x07u,     //drive parameter activity failed(hard disk)
	DMA_OVERRUN = 0x08u,     //DMA overrun
	DATA_BOUNDARY_ERROR = 0x09u,     //data boundary error(attempted DMA across 64K boundary or > 80h sectors)
	BAD_SECTOR_DETECTED_HD = 0x0Au,     //bad sector detected(hard disk)
	BAD_TRACK_DETECTED_HD = 0x0Bu,     //bad track detected(hard disk)
	UNSUPPORTED_TRACK_OR_INVALID_MEDIA = 0x0Cu,     //unsupported track or invalid media
	INVALID_NUM_SECTORS_ON_FORMAT_PS2HD = 0x0Du,     //invalid number of sectors on format(PS / 2 hard disk)
	CONTROL_DATA_ADDRESS_MARK_DETECTED_HD = 0x0Eu,     //control data address mark detected(hard disk)
	DMA_ARBITRATION_LEVEL_OUT_OF_RANGE_HD = 0x0Fu,     //DMA arbitration level out of range(hard disk)
	UNCORRECTABLE_CRC_OR_ECC_ERROR_ON_READ = 0x10u,     //uncorrectable CRC or ECC error on read
	DATA_ECC_CORRECTED_HD = 0x11u,     //data ECC corrected(hard disk)
	CONTROLLER_FAILURE = 0x20u,     //controller failure
	NO_MEDIA_IN_DRIVE_IBM_MS_INT13_EXT = 0x31u,     //no media in drive(IBM / MS INT 13 extensions)
	INCORRECT_DRIVE_TYPE_CMOS_COMPAQ = 0x32u,     //incorrect drive type stored in CMOS(Compaq)
	SEEK_FAILED = 0x40u,     //seek failed
	TIMEOUT_NOT_READY = 0x80u,     //timeout(not ready)
	DRIVE_NOT_READY_HD = 0xAAu,     //drive not ready(hard disk)
	VOLUME_NOT_LOCKED_DRIVE_INT13_EXT = 0xB0u,     //volume not locked in drive(INT 13 extensions)
	VOLUME_LOCKED_DRIVE_INT13_EXT = 0xB1u,     //volume locked in drive(INT 13 extensions)
	VOLUME_NOT_REMOVABLE_INT13_EXT = 0xB2u,     //volume not removable(INT 13 extensions)
	VOLUME_IN_USE_INT13_EXT = 0xB3u,     //volume in use(INT 13 extensions)
	LOCK_COUNT_EXCEEDED_INT13_EXT = 0xB4u,     //lock count exceeded(INT 13 extensions)
	VALID_EJECT_REQUEST_FAILED_INT13_EXT = 0xB5u,     //valid eject request failed(INT 13 extensions)
	VOLUME_PRESENT_BUT_READ_PROTECTED_INT13_EXT = 0xB6u,     //volume present but read protected (INT 13 extensions)
	UNDEFINED_ERROR_HD = 0xBBu,     //undefined error(hard disk)
	WRITE_FAULT_HD = 0xCCu,     //write fault(hard disk)
	STATUS_REGISTER_ERROR_HD = 0xE0u,     //status register error(hard disk)
	SENSE_OPERATION_FAILED_HD = 0xFFu    //sense operation failed(hard disk)
};

static inline auto DetectGeometry(std::uint64_t sectors_v)
-> VirtualBiosDisk::Geometry const&
{
	using G = VirtualBiosDisk::Geometry;
	static constexpr const G table_s[] = {
		/* 160K  */ {  320u, 40u, 1u,  8u },
		/* 180K  */ {  360u, 40u, 1u,  9u },
		/* 320K  */ {  640u, 40u, 2u,  8u },
		/* 360K  */ {  720u, 40u, 2u,  9u },
		/* 640K  */ { 1280u, 80u, 2u,  8u },
		/* 720K  */ { 1440u, 80u, 2u,  9u },
		/* 1.2M  */ { 2400u, 80u, 2u, 15u },
		/* 1.44M */ { 2880u, 80u, 2u, 18u },
		/* 2.88M */ { 5760u, 80u, 2u, 36u }
	};

	auto const iterator_v = std::lower_bound(
		std::begin(table_s), std::end(table_s),
		G{ sectors_v, 0, 0, 0 },
		[](auto const& lhs_v, auto const& rhs_v) {
			return lhs_v.SectorsLBA < rhs_v.SectorsLBA;
		});
	if (iterator_v == std::end(table_s)) {
		throw std::runtime_error(std::format(
			"Unable to detect geometry for {} sectors.",
			sectors_v));
	}
	return *iterator_v;
}

static inline auto CalculateOffset(VirtualBiosDisk::Geometry const& geom_v,
	VirtualBiosDisk::Index const& index_v, bool uselba_v) -> std::uintmax_t
{
	std::uintmax_t lba_v { index_v.SectorLBA };
	if (!uselba_v)
	{
		if((index_v.Sector - 1u) >= geom_v.Sectors
			||index_v.Head >= geom_v.Heads
			||index_v.Track >= geom_v.Tracks)
		{
			throw std::out_of_range(std::format(
				"Index out of range of geometry"));
		}
		lba_v = (index_v.Track - 1u) * geom_v.Heads * geom_v.Sectors +
			index_v.Head * geom_v.Sectors + (index_v.Sector - 1u);
	}
	if (lba_v >= geom_v.SectorsLBA) {
		throw std::out_of_range(std::format(
			"Index out of range of geometry"));
	}
	return lba_v * VirtualBiosDisk::kSectorSize;
}

VirtualBiosDisk::VirtualBiosDisk(Machine& machine_v, std::uint8_t drive_id_v)
	: m_Bridge13h(machine_v, 0x13u, 0x1u, *this)
	, m_Bridge19h(machine_v, 0x19u, 0x1u, *this)
	, m_DriveID(drive_id_v)
{}

auto VirtualBiosDisk::MountImage(std::filesystem::path const& path_v, Geometry const& geometry_v, bool use_chs_v) -> void
{
	using namespace std::filesystem;
	if (!exists(path_v) || file_size(path_v) == 0u) {
		throw std::system_error(std::make_error_code(
			std::errc::no_such_file_or_directory)),
			std::format("File {} - not found or empty.", path_v.string());
	}
	std::uint64_t number_of_sectors_v = file_size(path_v);
	number_of_sectors_v /= 512u;
	auto proposed_sectors_v = geometry_v.SectorsLBA;
	auto use_lba_v = true;
	if (true == use_chs_v) {
		proposed_sectors_v
			= geometry_v.Tracks
			* geometry_v.Heads
			* geometry_v.Sectors;
		use_lba_v = false;
	}

	if (proposed_sectors_v != number_of_sectors_v) {
		throw std::runtime_error(std::format(
			"Geometry for image {}, does not match.",
			path_v.string()));
	}
	//m_File.exceptions(std::ios::failbit);
	m_File.open(path_v, std::ios::binary|std::ios_base::in|std::ios_base::out);
	if (!m_File.is_open()) {
		throw std::system_error(std::make_error_code(std::errc::io_error),
			std::format("Unable to open file {}.", (current_path()/path_v).string()));
	}

	m_Geometry = geometry_v;
	m_UseLBA = use_lba_v;
}

auto VirtualBiosDisk::MountImage(std::filesystem::path const& path_v, bool use_chs_v) -> void
{
	if (!exists(path_v) || file_size(path_v) < VirtualBiosDisk::kSectorSize) {
		throw std::system_error(std::make_error_code(
			std::errc::no_such_file_or_directory)),
			std::format("File {} - not found or empty.", path_v.string());
	}
	std::uint64_t number_of_sectors_v = file_size(path_v);
	number_of_sectors_v /= VirtualBiosDisk::kSectorSize;
	VirtualBiosDisk::Geometry geometry_v { number_of_sectors_v, 0, 0, 0 };
	if (true == use_chs_v) geometry_v = DetectGeometry(number_of_sectors_v);
	return VirtualBiosDisk::MountImage(path_v, geometry_v, use_chs_v);
}

auto VirtualBiosDisk::Unmount() -> void
{
	m_File.close();
	m_Geometry = { 0, 0, 0, 0 };
	m_UseLBA = true;
}

auto core::VirtualBiosDisk::VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& R, std::uint16_t callno_v) -> bool
{
	switch (callno_v) {
	case 0x13u: return Int13h(machine_v, cpuindex_v, R);
	case 0x19u: return Int19h(machine_v, cpuindex_v, R);
	default:
		return false;
	}
}

auto VirtualBiosDisk::Int13h(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& R) -> bool 
{
	switch (R.ah) {
	case 0x00u:
		if (R.dl != m_DriveID)
		{
			R.flags |= 0x1u;
			return false;
		}
		R.flags &= ~0x1u;
		return true;
	case 0x01u:
	default:
		__debugbreak();
		break;
	}
	return false;
}

auto VirtualBiosDisk::Int19h(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& R) -> bool 
{
	if (!m_File.is_open()) {
		throw std::runtime_error("Unable to boot, no boot disk mounted.");
	}

	std::uint64_t address_v { 0x7C00u };
	std::vector<std::byte> buffer_v (kSectorSize);
	std::span<std::byte> buffer_s{ buffer_v };

	if (Fetch(buffer_s, Index{ .SectorLBA = 0u }) < kSectorSize) {
		throw std::runtime_error("Unable to boot, I/O error.");
	}
	if (WHvTranslateGvaResultSuccess!=
		machine_v.TranslateVirtualAddress(cpuindex_v, address_v)) {
		throw std::runtime_error("Unable to locate address 0x7C00u");
	}
	machine_v.WritePhysical(cpuindex_v, address_v, buffer_s);	
	R.cs = 0x0000u;
	R.cs_base = 0x0000u;
	R.cs_size = 0xFFFFu;
	R.rip = 0x7c00u;
	R.dh = m_DriveID;
	return true;
}

auto VirtualBiosDisk::Fetch(std::span<std::byte>& buffer_v, Index const& index_v, bool uselba_v) -> std::size_t {
	auto offset_v = CalculateOffset(m_Geometry, index_v, uselba_v);
	m_File.seekg(offset_v);
	m_File.read((char*)buffer_v.data(), buffer_v.size());
	auto const bytes_consumed_v = m_File.gcount();
	buffer_v = buffer_v.subspan(0, bytes_consumed_v);
	return bytes_consumed_v;
}

auto VirtualBiosDisk::Write(std::span<std::byte const>& buffer_v, Index const& index_v, bool uselba_v) -> std::size_t {
	auto offset_v = CalculateOffset(m_Geometry, index_v, uselba_v);
	m_File.seekp(offset_v);
	m_File.write((char const*)buffer_v.data(), buffer_v.size());
	auto const bytes_consumed_v = m_File.gcount();
	buffer_v = buffer_v.subspan(bytes_consumed_v);
	return bytes_consumed_v;
}
