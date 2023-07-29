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
	, m_Data      { nullptr }
	, m_Base      { 0 }
	, m_Size      { 0 }
{
	if (size_v < 1u) {
		throw std::invalid_argument(
			"Must be atleast one page");
	}
	if (size_v > kPageLimit) {
		throw std::invalid_argument(
			"Size must be less than 2^48");
	}
	if (base_v + size_v > kPageLimit) {
		throw std::invalid_argument(
			"Base + size must be less than 2^48");
	}

	base_v *= kPageSize;
	size_v *= kPageSize;

	m_Data = (std::byte*)::VirtualAlloc(nullptr, size_v,
		MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (nullptr==m_Data) {
		win32::error::throw_last_error();
	}	

	WIN32_ERROR_ASSERT(partition_v.MapGpaRange(m_Data, base_v, size_v, prot_v));

	m_Base = base_v;
	m_Size = size_v;	
}

using std::exchange;

Memory::Memory(Memory&& from_v) noexcept
	: m_Partition{ from_v.m_Partition }
	, m_Size{ exchange(from_v.m_Size, 0) }
	, m_Base{ exchange(from_v.m_Base, 0) }
	, m_Data{ exchange(from_v.m_Data, nullptr) }
{

}

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
	std::swap(m_Size, other_v.m_Size);
	std::swap(m_Base, other_v.m_Base);
	std::swap(m_Data, other_v.m_Data);
}

Memory::~Memory()
{
	if (m_Data) {
		m_Partition->UnmapGpaRange(m_Base, m_Size);
		::VirtualFree(m_Data, 0, MEM_RELEASE);
	}
}

auto Memory::Base() const noexcept -> std::uint64_t
{
	return m_Base / kPageSize;
}

auto Memory::Size() const noexcept -> std::uint64_t
{
	return m_Size / kPageSize;
}

auto Memory::Data() const noexcept -> std::byte*
{
	return m_Data;
}

auto Memory::Load(std::filesystem::path path_v, std::uint64_t dst_offset_v, std::uint64_t src_offset_v, std::uint64_t src_length_v) -> std::size_t
{
	using utils::logger;
	path_v = utils::path_substitute(path_v);
	if (!m_Data) throw std::runtime_error("Memory is not allocated");	
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
		m_Size - dst_offset_v));
	if (src_length_v < 1u)
		return 0u;		
	logger::trace(logger::deflog, "{}: path_v={} dst_offset_v={:#x} src_offset_v={:#x} src_length_v={:#x} base={:#x} size={:#x}",
		__func__, path_v.string(), dst_offset_v, src_offset_v, src_length_v, m_Base, m_Size);
	std::ifstream file_v{ path_v, std::ios::binary };
	if (!file_v.is_open())
		throw std::system_error(
			std::make_error_code(std::errc::io_error),
			path_v.string());
	file_v.seekg(src_offset_v, std::ios::beg);
	file_v.read ((char*)m_Data + dst_offset_v, src_length_v);
	return file_v.gcount();	
}
