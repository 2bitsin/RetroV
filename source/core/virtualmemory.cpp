#include <core/virtualmemory.hpp>
#include <core/constants.hpp>
#include <win32/whvpartition.hpp>
#include <utils/paths.hpp>

#include <system_error>
#include <fstream>

using core::VirtualMemory;

VirtualMemory::VirtualMemory(win32::WHvPartition const& partition_v, std::uint64_t base_v, std::uint64_t size_v, Access prot_v)
	: m_Partition { partition_v }
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

VirtualMemory::~VirtualMemory()
{
	if (m_Data) {
		m_Partition.UnmapGpaRange(m_Base, m_Size);
		WIN32_ERROR_ASSERT(::VirtualFree(m_Data, 0, MEM_RELEASE));
	}
}

auto VirtualMemory::Base() const noexcept -> std::uint64_t
{
	return m_Base / kPageSize;
}

auto VirtualMemory::Size() const noexcept -> std::uint64_t
{
	return m_Size / kPageSize;
}

auto VirtualMemory::Data() const noexcept -> std::byte*
{
	return m_Data;
}

auto VirtualMemory::Load(std::filesystem::path path_v) -> std::size_t
{
	path_v = utils::path_substitute(path_v);

	if (!m_Data) {
		throw std::runtime_error(
			"Memory is not allocated");
	}

	if (!std::filesystem::exists(path_v)) 
		throw std::system_error( 
			std::make_error_code(std::errc::no_such_file_or_directory),
			path_v.string());
	
	if (std::filesystem::is_directory(path_v))
		throw std::system_error(
			std::make_error_code(std::errc::is_a_directory),
			path_v.string());

	auto const file_size_v  = std::filesystem::file_size(path_v);
	
	if (file_size_v > m_Size)
		throw std::system_error(
			std::make_error_code(std::errc::file_too_large),
			path_v.string());

	if (file_size_v < 1u)
		throw std::invalid_argument(
			"File must be atleast one byte");

	std::ifstream file_v{ path_v, std::ios::binary };

	if (!file_v.is_open())
		throw std::system_error(
			std::make_error_code(std::errc::io_error),
			path_v.string());

	file_v.read ((char*)m_Data, file_size_v);

	if (file_v.gcount() != file_size_v)
		throw std::system_error(
			std::make_error_code(std::errc::io_error),
			path_v.string());
	
}
