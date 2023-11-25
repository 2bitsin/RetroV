#include <core/Memory.hpp>
#include <core/Machine.hpp>

#include <core/configuration.hpp>
#include <core/constants.hpp>

#include <utils/algorithm.hpp>
#include <utils/literals.hpp>
#include <utils/region.hpp>
#include <utils/logger.hpp>

using core::Memory;

using namespace size_literals;

Memory::Memory(Machine& machine_v)
	: m_Machine{ machine_v }
{}

auto Memory::ConfigureMemory(Configuration const& config_v) -> void
{
	static constexpr utils::region64_type ram_map_s[] =
	{
		{ utils::from_range, 0x00000000u, 0x000A0000u }, // Coventional memory
	//{ utils::from_range, 0x000A0000u, 0x00100000u }, // ROM Area
		{ utils::from_range, 0x00100000u, 0x00200000u }, // A20 memory mirror 
		{ utils::from_range, 0x00200000u, 0x00F00000u }, // Extended memory under 15MiB
	//{ utils::from_range, 0x00F00000u, 0x01000000u }, // ISA memory hole
		{ utils::from_range, 0x01000000u, 0xC0000000u }, // Extended memory over 15MiB
	//{ utils::from_range, 0xC0000000u, 0xFEE00000u }, // PCI memory hole
	//{ utils::from_range, 0xFEE00000u, 0xFEE01000u }, // Local APIC
	//{ utils::from_range, 0xFEE01000u, 0xFFE00000u }, // Unused ?
	//{ utils::from_range, 0xFFE00000u, 0xFFEFFFFFu }, // High part of BIOS ROM
	// Remaining
		{ utils::from_range, 0x0000000100000000u, 0xFFFFFFFFFFFFFFFFu },
	};

	static constexpr utils::region64_type empty_regions_s[] = {
		{ utils::from_range, 0x000A0000u, 0x00100000u }
	};

	static constexpr alignas(kPageSize) auto default_page_s = utils::make_filled_array<uint8_t, kPageSize>(0xFFu);

	auto& partition_v = m_Machine.GetPartition();
	WIN32_ERROR_ASSERT(partition_v.Reset());

	m_MainMemory = win32::VirtualAlloc_s(
		config_v.GetPropertyUint64("system.memory.size.megabytes") * 1_MiB,
		win32::execute_read_write);

	auto memory_v = std::span(m_MainMemory);
	for (auto&& window_v : ram_map_s)
	{
		if (memory_v.empty()) break;
		auto slice_v = utils::take_slice(memory_v, window_v.size());
		m_MappedRanges.emplace_back(partition_v, window_v, kAccessMemory, slice_v);
	}

	for (auto&& region_v : empty_regions_s)
		for (auto curr_page_v = region_v.begin();
			curr_page_v < region_v.end();
			curr_page_v += default_page_s.size())
	{
		if (curr_page_v >= 0xA0000u && curr_page_v < 0xC0000u) continue;
		WIN32_ERROR_ASSERT(partition_v.MapGpaRange(default_page_s.data(), curr_page_v,
			default_page_s.size(), kAccessReadOnly));
	}

	// Uninitialized memory read trap
	// So we can catch unimplemented BDA/interrupt access
	//WIN32_ERROR_ASSERT(partition_v.UnmapGpaRange(0, 0x1000u));
}

auto Memory::ConfigureBiosROM(Configuration const& config_v) -> void
{

	auto const path_v = config_v.GetPropertyString("system.rom.path");
	static constexpr auto const region_lo = RomImage::region_type{ utils::size_invert, 1_MiB, 192_KiB };
	static constexpr auto const check_lo = RomImage::validate{ 4_KiB, 1u, region_lo.size() / 4_KiB };
	static constexpr auto const region_hi = RomImage::region_type{ utils::size_invert, 4_GiB, 16_MiB };
	static constexpr auto const check_hi = RomImage::validate{ 4_KiB, 1u, region_hi.size() / 4_KiB };
	static constexpr auto const align_v = RomImage::kTopAligned;
	auto& partition_v = m_Machine.GetPartition();
	m_MappedRoms.emplace_back(partition_v, check_lo, path_v, region_lo, align_v);
	m_MappedRoms.emplace_back(partition_v, check_hi, path_v, region_hi, align_v);
}


auto Memory::FetchMemory(core::Processor const& vcpu_v, uint64_t address_v, uint64_t length_v, std::vector<std::byte>& output_v) -> int32_t
{
	using std::tie;
	
	uint64_t page_v = address_v & ~0xFFFu;
	uint64_t offs_v = address_v & 0xFFFu;
	auto [status_v, xgpa_v] = vcpu_v.TranslateAddress(page_v, Access::kAccessFetch);
	auto const zero_terminated_v = length_v == 0u;
	// Force wrap around to max length
	if (!zero_terminated_v) {
		output_v.reserve(output_v.size() + length_v);
	}
	else {
		length_v -= 1u;
	}

	size_t max_bytes_v{ 0 };
	std::byte tmpbuf_v[16u]{ std::byte(0) };

	while (true)
	{
		max_bytes_v = std::min(std::min(length_v, sizeof(tmpbuf_v)), 0x1000u - offs_v);
		auto buffer_s = utils::as_static_mutable_bytes(tmpbuf_v).first(max_bytes_v);
		status_v = vcpu_v.MemoryAccess(false, xgpa_v + offs_v, buffer_s);

		if (status_v != ERROR_SUCCESS)
			return status_v;

		if (!zero_terminated_v)
			output_v.insert(output_v.end(), tmpbuf_v,
				tmpbuf_v + buffer_s.size());
		else
			for (auto byte_v : tmpbuf_v) {
				if (byte_v == std::byte(0))
					goto Done;
				output_v.push_back(byte_v);
			}

		length_v -= buffer_s.size();
		if (length_v < 1u) break;
		offs_v += buffer_s.size();
		if (offs_v >= 0x1000u) {
			offs_v &= 0xFFFu; page_v += 0x1000u;
			tie(status_v, xgpa_v) = vcpu_v.TranslateAddress(page_v, Access::kAccessFetch);
			if (status_v != ERROR_SUCCESS)
				return status_v;
		}
	}
Done:
	return ERROR_SUCCESS;
}

auto Memory::MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v) -> int32_t
{
	/*************************************************************************
	 *
	 *  SIMPLE MECHANISM TO CATCH READS OF UNINITIALIZED MEMORY WITHIN PAGE 0
	 *  (BIOS DATA AREA, INTERRUPT VECTOR TABLE, ETC)
	 *
	 *************************************************************************/

	if (physaddr_v < 0x1000u)
	{
		assert(physaddr_v + data_v.size() < 0x1000u);
		if (is_write_v)
		{
			std::memcpy(m_MainMemory.data(), data_v.data(), data_v.size());
			for (auto address_v = physaddr_v; address_v < physaddr_v + data_v.size(); address_v += 1u)
				m_PageZeroStatus.set(address_v, true);
			return ERROR_SUCCESS;
		}

		for (auto address_v = physaddr_v; address_v < physaddr_v + data_v.size(); address_v += 1u)
			if (!m_PageZeroStatus.test(address_v)) {
				using utils::logger;
				logger::debug(logger::deflog, "Reading uninitialized memory at 0x{:08x}", address_v);
				__debugbreak();
			}
		std::memcpy(data_v.data(), m_MainMemory.data(), data_v.size());
		return ERROR_SUCCESS;
	}

	/**************************
	 *
	 *  END OF PAGE 0 READ TRAP
	 *
	 **************************/

	return ERROR_ACCESS_DENIED;
}

