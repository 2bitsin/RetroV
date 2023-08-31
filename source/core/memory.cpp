#include <core/memory.hpp>
#include <core/constants.hpp>
#include <win32/whvpartition.hpp>
#include <utils/paths.hpp>
#include <utils/logger.hpp>

#include <system_error>
#include <fstream>
#include <utility>

using core::Memory;
using namespace win32;

Memory::Memory(win32::WHvPartition const& partition_v, std::uint64_t base_v, std::uint64_t size_v, Access flags_v)
	: m_Partition { &partition_v }
	, m_Data      { virtual_alloc_s(size_v, 
			page_prot::execute_read_write, 
			alloc_flag::reserve|
			alloc_flag::commit|
			alloc_flag::write_watch) }
	, m_Base      { base_v }
	, m_Flags     { flags_v }
	, m_Dirty     { }
{
	using utils::logger;
	m_Dirty.resize((size_v + kPageSize - 1) / kPageSize);
	WIN32_ERROR_ASSERT(Map());
}

using std::exchange;

Memory::Memory(Memory&& from_v) noexcept
	: m_Partition{ from_v.m_Partition }	
	, m_Base{ exchange(from_v.m_Base, 0) }
	, m_Data{ exchange(from_v.m_Data, {})}
	, m_Flags{ exchange(from_v.m_Flags, {}) }
	, m_Dirty{ exchange(from_v.m_Dirty, {}) }
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
	std::swap(m_Flags, other_v.m_Flags);
	std::swap(m_Dirty, other_v.m_Dirty);
}

Memory::~Memory()
{
	using utils::logger;
	Unmap();
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
	s_log.MapGpaRangeFromFile(m_Data.data(), m_Base+dst_offset_v, 
		std::min(src_length_v, m_Data.size()),
		path_v, src_offset_v, src_length_v);
	std::ifstream file_v{ path_v, std::ios::binary };
	if (!file_v.is_open())
		throw std::system_error(
			std::make_error_code(std::errc::io_error),
			path_v.string());
	file_v.seekg(src_offset_v, std::ios::beg);
	file_v.read ((char*)m_Data.data() + dst_offset_v, src_length_v);
	return file_v.gcount();	
}

auto Memory::CopyDirtyPagesTo(std::span<std::byte> target_v) -> std::int32_t 
{
	using namespace win32;	
	auto[status_v, gran_v, list_v] = query_and_reset_dirty_pages(
		m_Data.data(), m_Data.size(), m_Dirty, true);
	if (FAILED(status_v)) 
		return status_v;
	for (auto&& page_base_v : list_v) {
		auto const offset_v = (std::byte const*)page_base_v - m_Data.data();
		std::memcpy(target_v.data() + offset_v, page_base_v, gran_v);
	}
	return S_OK;
}

auto Memory::Unmap() noexcept -> std::int32_t
{
	if (m_Data.empty())
		return S_OK;
	s_log.UnmapGpaRange(m_Base, m_Data.size());
	return m_Partition->UnmapGpaRange(m_Base, m_Data.size());
}

auto Memory::Map() noexcept -> std::int32_t
{
	using utils::logger;
	if (m_Data.empty())
		return S_OK;
	s_log.MapGpaRange(m_Data.data(), m_Base, m_Data.size(), m_Flags);
	return m_Partition->MapGpaRange(m_Data.data(), m_Base, m_Data.size(), m_Flags);
}
