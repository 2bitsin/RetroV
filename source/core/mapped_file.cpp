#include <core/mapped_file.hpp>
#include <core/access.hpp>

#include <algorithm>
#include <thread>
#include <mutex>

static auto inline block_size() -> std::uint64_t
{
	static std::uint64_t _Granularity { 0 };
	static std::once_flag once_v;

	std::call_once(once_v, [] () { 
		SYSTEM_INFO info_v { 0 };
		::GetSystemInfo(&info_v); 
		_Granularity = std::min<std::uint64_t>(core::aligned_memory::alignment(),
			info_v.dwAllocationGranularity);
	});

	return _Granularity;
}


core::mapped_file::mapped_file(std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v, std::uint32_t access_v)
	: m_file(INVALID_HANDLE_VALUE)
	, m_view(INVALID_HANDLE_VALUE)
	, m_base(nullptr)
	, m_buffer(nullptr)
	, m_length(0)
	, m_access(0)
{

	if(!(access_v&core::access::write) && !std::filesystem::exists(path_v))
		throw std::runtime_error("file does not exist");
	auto const current_file_size_v = std::filesystem::file_size(path_v);

	if(length_v==0)length_v=current_file_size_v;

	auto disposition_v = 0u;
	auto desired_access_v = 0u;
	auto map_access_v = 0u;
	
	auto const block_size_v = block_size();
	auto const aligned_offset_v = offset_v & ~(block_size_v - 1);
	auto const aligned_length_v = ((offset_v + length_v + block_size_v - 1) & ~(block_size_v - 1)) - aligned_offset_v;

	auto desired_file_size_v = current_file_size_v;
	if (access_v&core::access::write) {
		desired_file_size_v = std::max(aligned_length_v+aligned_offset_v, current_file_size_v);
	}

	auto const protect_v = core::protect_from_access (access_v);
	if (access_v&core::access::write  ) disposition_v = OPEN_ALWAYS; else disposition_v = OPEN_EXISTING;
	if (access_v&core::access::read   ) desired_access_v|=GENERIC_READ; 	
	if (access_v&core::access::write  ) desired_access_v|=GENERIC_WRITE; 	
	if (access_v&core::access::execute) desired_access_v|=GENERIC_EXECUTE; 
	if (access_v&core::access::write  ) map_access_v|=FILE_MAP_WRITE;
	if (access_v&core::access::read   ) map_access_v|=FILE_MAP_READ;
	if (access_v&core::access::execute) map_access_v|=FILE_MAP_EXECUTE;

	auto const file_v = ::CreateFileW(path_v.c_str(), desired_access_v, FILE_SHARE_READ,
		nullptr, disposition_v, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file_v == INVALID_HANDLE_VALUE)
		throw win32_error();

	auto const view_v = ::CreateFileMappingW(file_v, nullptr, protect_v, desired_file_size_v>>32u,
		desired_file_size_v&0xFFFFFFFFu, nullptr);
	if (view_v==INVALID_HANDLE_VALUE||view_v==nullptr)
		throw win32_error();
	
	auto const base_v = (std::byte*)::MapViewOfFile(view_v, map_access_v, aligned_offset_v >> 32u,
		aligned_offset_v&0xFFFFFFFFu, aligned_length_v);

	if (base_v==nullptr)throw win32_error();

	m_file   = file_v;
	m_view   = view_v;
	m_base   = base_v;
	m_buffer = m_base + (offset_v - aligned_offset_v);
	m_length = length_v;
	m_access = access_v;
}

core::mapped_file::~mapped_file()
{
	while (!m_unmaps.empty()) {
		m_unmaps.back() ();
		m_unmaps.pop_back();
	}

	if (m_base != nullptr) ::UnmapViewOfFile(m_base);
	if (m_view != INVALID_HANDLE_VALUE) ::CloseHandle(m_view);
	if (m_file != INVALID_HANDLE_VALUE) ::CloseHandle(m_file);

	m_base = nullptr;
	m_view = INVALID_HANDLE_VALUE;
	m_file = INVALID_HANDLE_VALUE;
	m_buffer = nullptr;
	m_length = 0;
	m_access = 0;
}

auto core::mapped_file::attach_to(core::hypervisor& host_v, std::uint64_t base_v, std::uint64_t size_v) -> void
{
	size_v = size_v>0u?std::min(size_v, m_length):m_length;
	host_v.map_physical_memory(m_buffer, base_v, size_v, m_access);	
	m_unmaps.push_back([&host_v, base_v, size_v] () {
		host_v.unmap_physical_memory(base_v, size_v);
	});
}

auto core::mapped_file::lock_mutable(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte>
{
	if (size_v == 0)size_v = m_length;
	if (base_v >= m_length) throw std::invalid_argument("invalid range");
	if (base_v + size_v > m_length) throw std::invalid_argument("invalid range");
	if (!m_locked.try_lock()) throw std::logic_error("file is already locked");
	return std::span<std::byte>(m_buffer + base_v, size_v);	
}

auto core::mapped_file::lock_constant(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte const>
{
	if (size_v == 0)size_v = m_length;
	if (base_v >= m_length) throw std::invalid_argument("invalid range");
	if (base_v + size_v > m_length) throw std::invalid_argument("invalid range");
	if (!m_locked.try_lock()) throw std::logic_error("file is already locked");
	return std::span<std::byte const>(m_buffer + base_v, size_v);
}

auto core::mapped_file::unlock() -> void
{
	m_locked.unlock();
}

auto core::mapped_file::size() const -> std::uint64_t
{
	return m_length;
}
