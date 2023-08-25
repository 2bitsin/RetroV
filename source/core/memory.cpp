#include <core/memory.hpp>
#include <core/constants.hpp>
#include <win32/whvpartition.hpp>
#include <utils/paths.hpp>
#include <utils/logger.hpp>

#include <system_error>
#include <fstream>
#include <utility>

using core::Memory;

Memory::Memory(win32::WHvPartition const& partition_v, std::uint64_t base_v, std::uint64_t size_v, Access prot_v)
	: m_Partition { &partition_v }
	, m_Data      {   }
	, m_Base      { 0 }
	, m_Dirty     {   }
{
	using utils::logger;

	if (size_v < kPageSize) {
		throw std::invalid_argument(
			"Must be atleast one page");
	}

	if ((base_v + size_v) < std::max(base_v, size_v)) {
		throw std::invalid_argument(
			"Base + size overflows 64bit");
	}

	auto data_v = win32::virtual_alloc_s(size_v, win32::execute_read_write);	

	logger::trace(logger::deflog, "Mapping {:#016x} ... {:#016x} -> {:#016x} | {:#04b}", 
		base_v, base_v+size_v, (std::uintptr_t)m_Data.data(), (std::uint32_t)prot_v);
	WIN32_ERROR_ASSERT(partition_v.MapGpaRange(m_Data.data(), base_v, size_v, prot_v));

	m_Data = std::move (data_v);
	m_Base = std::move (base_v);
}

using std::exchange;

Memory::Memory(Memory&& from_v) noexcept
	: m_Partition{ from_v.m_Partition }	
	, m_Base{ exchange(from_v.m_Base, 0) }
	, m_Data{ exchange(from_v.m_Data, {})}
{}

auto Memory::operator=(Memory&& from_v) noexcept -> Memory&
{
	if (&from_v != this) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}

	return *this;
}

auto Memory::swap(Memory& other_v) noexcept -> void 
{
	std::swap(m_Partition, other_v.m_Partition);
	std::swap(m_Base, other_v.m_Base);
	std::swap(m_Data, other_v.m_Data);
}

Memory::~Memory()
{
	using utils::logger;

	if (m_Data.empty()) 
		return;

	m_Partition->UnmapGpaRange(m_Base, m_Data.size());
	logger::trace(logger::deflog, "Unmapping {:#016x} ... {:#016x}", m_Base, m_Base+m_Data.size());	
}

auto Memory::Base() const noexcept -> std::uint64_t { return m_Base; }
auto Memory::Size() const noexcept -> std::uint64_t { return m_Data.size(); }
auto Memory::Data() const noexcept -> std::span<std::byte> { return m_Data; }

auto Memory::Load(std::filesystem::path path_v, std::uint64_t dst_offset_v, 
	std::uint64_t src_offset_v, std::uint64_t src_length_v) -> std::size_t 
{
	using utils::logger;
	path_v = utils::path_substitute(path_v);
	if (m_Data.empty()) 
		throw std::runtime_error("Memory is not allocated");
	if (!std::filesystem::exists(path_v)) 
		throw std::system_error( 
			std::make_error_code(std::errc::no_such_file_or_directory),
			path_v.string());	
	if (std::filesystem::is_directory(path_v))
		throw std::system_error(
			std::make_error_code(std::errc::is_a_directory),
			path_v.string());
	auto const file_size_v  = std::filesystem::file_size(path_v);
	if (file_size_v < 1u)
		throw std::invalid_argument(
			"File must be atleast one byte");
	if (src_offset_v >= file_size_v)
		throw std::invalid_argument(
			"Source offset is out of range");
	src_length_v = std::min(src_length_v, std::min(
		file_size_v - src_offset_v, 
		m_Data.size() - dst_offset_v));
	if (src_length_v < 1u)
		return 0u;		
	logger::trace(logger::deflog, 
		"Mapping {:#016x} ... {:#016x} -> {} @ {:#016x} ... {:#016x}",
		m_Base + src_offset_v, 
		m_Base + src_offset_v + m_Data.size(),
		path_v.string(),
		dst_offset_v, 
		src_length_v
	);
	std::ifstream file_v{ path_v, std::ios::binary };
	if (!file_v.is_open())
		throw std::system_error(
			std::make_error_code(std::errc::io_error),
			path_v.string());
	file_v.seekg(src_offset_v, std::ios::beg);
	file_v.read ((char*)m_Data.data() + dst_offset_v, src_length_v);
	return file_v.gcount();	
}

auto Memory::CopyDirtyPagesTo(std::span<std::byte> target_v) -> std::int32_t {
	using namespace win32;
	auto const required_dwords_v = ((m_Data.size() / kPageSize) + 63u) / 64u;
	if (m_Dirty.size() < required_dwords_v) m_Dirty.resize(required_dwords_v);		
  auto status_v = m_Partition->QueryGpaRangeDirtyBitmap(m_Base, m_Data.size(), m_Dirty);
	if (FAILED(status_v)) return status_v;
	return copy_dirty_pages(target_v, m_Data, m_Dirty);
}
