#include <win32/mappedfile.hpp>
#include <utils/algorithm.hpp>

using win32::MappedFile;

MappedFile::MappedFile(
	std::filesystem::path const& path_v,
	utils::region64_type regn_v, 
	cf_mode mode_v,
	page_prot prot_v, 
	share_type share_v)
{
	unsigned long access_v = 0u;
	unsigned long m_prot_v = 0u;

	if(prot_v & page_prot::execute_read_write) access_v |= GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE;
	if(prot_v & page_prot::execute_write_copy) access_v |= GENERIC_READ|GENERIC_EXECUTE;
	if(prot_v & page_prot::execute_read) access_v |= GENERIC_READ|GENERIC_EXECUTE;
	if(prot_v & page_prot::read_write) access_v |= GENERIC_READ|GENERIC_WRITE;
	if(prot_v & page_prot::write_copy) access_v |= GENERIC_READ;
	if(prot_v & page_prot::read_only) access_v |= GENERIC_READ;	

	if(prot_v & page_prot::execute_read_write) m_prot_v |= FILE_MAP_EXECUTE|FILE_MAP_WRITE|FILE_MAP_READ;
	if(prot_v & page_prot::execute_write_copy) m_prot_v |= FILE_MAP_EXECUTE|FILE_MAP_READ|FILE_MAP_COPY;
	if(prot_v & page_prot::execute_read) m_prot_v |= FILE_MAP_EXECUTE|FILE_MAP_READ;
	if(prot_v & page_prot::read_write) m_prot_v |= FILE_MAP_WRITE|FILE_MAP_READ;
	if(prot_v & page_prot::write_copy) m_prot_v |= FILE_MAP_READ|FILE_MAP_COPY;
	if(prot_v & page_prot::read_only) m_prot_v |= FILE_MAP_READ;

	auto const wpath_v = path_v.wstring();

	unique_handle file_handle_v{ ::CreateFileW(wpath_v.c_str(), access_v, share_v, nullptr, mode_v, FILE_ATTRIBUTE_NORMAL, nullptr) };

	if(INVALID_HANDLE_VALUE == file_handle_v.get()) 
		error::throw_last_error();

	SYSTEM_INFO sysinfo_v { };
	::GetSystemInfo(&sysinfo_v);

	if (0 == regn_v.size()) {
		::LARGE_INTEGER size_v;
		if (!::GetFileSizeEx(file_handle_v.get(), &size_v))
			error::throw_last_error();
		regn_v.resize(size_v.QuadPart);
	}	

	auto round_regn_v = regn_v.round_outside_new(sysinfo_v.dwAllocationGranularity);

	round_regn_v.clamp(regn_v.end());

	auto const [size_hi_v, size_lo_v] = utils::integral_split_msw<std::uint32_t>(round_regn_v.end());

	unique_handle mapp_handle_v{ ::CreateFileMappingW(file_handle_v.get(), nullptr, prot_v, size_hi_v, size_lo_v, nullptr) };

	if (INVALID_HANDLE_VALUE == mapp_handle_v.get())
		error::throw_last_error();

	auto const [fileoff_hi_v, fileoff_lo_v] = utils::integral_split_msw<std::uint32_t>(round_regn_v.last_sync_time());

	auto const mapoffset_v = regn_v.last_sync_time() - round_regn_v.last_sync_time();

	m_MapPtr = (std::byte*)::MapViewOfFile(mapp_handle_v.get(), m_prot_v, fileoff_hi_v, fileoff_lo_v, round_regn_v.size());

	if (nullptr == m_MapPtr) error::throw_last_error();

	m_Data = std::span{ m_MapPtr + mapoffset_v, regn_v.size() };		
	m_File = std::move(file_handle_v);
	m_Mapp = std::move(mapp_handle_v);
}

MappedFile::MappedFile(MappedFile&& from_v) noexcept
	: m_File   { std::exchange(from_v.m_File,   nullptr) }
	, m_Mapp   { std::exchange(from_v.m_Mapp,   nullptr) }
	, m_MapPtr { std::exchange(from_v.m_MapPtr, nullptr) }
	, m_Data   { std::exchange(from_v.m_Data,   {})}
{}

auto MappedFile::operator=(MappedFile&& from_v) noexcept -> MappedFile&
{    
	if (this != &from_v) {
		MappedFile tmp_v{ std::move(from_v) };
		tmp_v.swap(*this);
	}
	return *this;
}

auto MappedFile::swap(MappedFile& other_v) noexcept -> void
{
	std::swap(m_File, other_v.m_File);
	std::swap(m_Mapp, other_v.m_Mapp);
	std::swap(m_MapPtr, other_v.m_MapPtr);
	std::swap(m_Data, other_v.m_Data);
}

auto MappedFile::Data() const noexcept -> std::span<std::byte>
{
	return m_Data;
}

auto MappedFile::Size() const noexcept -> std::size_t
{
	return m_Data.size();
}
