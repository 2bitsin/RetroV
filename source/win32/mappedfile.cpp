#include <win32/mappedfile.hpp>
#include <utils/algorithm.hpp>

using win32::MappedFile;

MappedFile::MappedFile(
	std::filesystem::path const& path_v,
	utils::region_64_t regn_v, 
	cf_mode mode_v,
	page_prot prot_v, 
	share_type share_v)
{
	unsigned long access_v = 0u;

	if(prot_v & page_prot::execute_read_write) access_v |= GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE;
	if(prot_v & page_prot::execute_write_copy) access_v |= GENERIC_READ|GENERIC_EXECUTE;
	if(prot_v & page_prot::execute_read) access_v |= GENERIC_READ|GENERIC_EXECUTE;
	if(prot_v & page_prot::read_write) access_v |= GENERIC_READ|GENERIC_WRITE;
	if(prot_v & page_prot::write_copy) access_v |= GENERIC_READ;
	if(prot_v & page_prot::read_only) access_v |= GENERIC_READ;	

	auto const wpath_v = path_v.wstring();

	unique_handle handle_v{ ::CreateFileW(wpath_v.c_str(), access_v,
		share_v, nullptr, mode_v, FILE_ATTRIBUTE_NORMAL, nullptr) };

	if(INVALID_HANDLE_VALUE == handle_v.get()) 
		error::throw_last_error();

	SYSTEM_INFO sysinfo_v { };
	::GetSystemInfo(&sysinfo_v);

	if (0 == regn_v.size()) {
		::LARGE_INTEGER size_v;
		if (!::GetFileSizeEx(handle_v.get(), &size_v))
			error::throw_last_error();
		regn_v.clamp(size_v.QuadPart);
	}	

	auto round_regn_v = regn_v.round_outside_new(
		sysinfo_v.dwAllocationGranularity);

	round_regn_v.clamp(regn_v.end());

	auto size_lo_v = (round_regn_v.end() >> 0x00u)&0xffffffffu;
	auto size_hi_v = (round_regn_v.end() >> 0x20u)&0xffffffffu;

	unique_handle map_handle_v{ ::CreateFileMappingW(
		handle_v.get(), nullptr, prot_v, size_hi_v, size_lo_v, nullptr) };

	if (INVALID_HANDLE_VALUE == map_handle_v.get())
		error::throw_last_error();

	auto const fileoff_lo_v = (round_regn_v.base() >> 0x00u)&0xffffffffu;
	auto const fileoff_hi_v = (round_regn_v.base() >> 0x20u)&0xffffffffu;

	auto const mapoffset_v = regn_v.base() - round_regn_v.base();

	m_MapPtr = (std::byte*)::MapViewOfFile(map_handle_v.get(),
		prot_v, fileoff_hi_v, fileoff_lo_v, round_regn_v.size());

	if (nullptr == m_MapPtr) error::throw_last_error();

	m_Data = std::span{ m_MapPtr + mapoffset_v, regn_v.size() };				
}

auto MappedFile::Data() const noexcept -> std::span<std::byte>
{
	return m_Data;
}

auto MappedFile::Size() const noexcept -> std::size_t
{
	return m_Data.size();
}
