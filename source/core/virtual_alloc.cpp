#include <core/virtual_alloc.hpp>

core::virtual_alloc_buffer::virtual_alloc_buffer(std::uint64_t size_v, std::uint32_t access_v)
	: m_buffer(nullptr)
	, m_length(0u)
	, m_access(0u)
{
	if (0 == size_v) throw std::invalid_argument("Can't allocate 0 bytes of memory.");

	std::uint32_t flags_v{ core::protect_from_access(access_v) };	

	auto length_v = (size_v + alignment() - 1) & ~(alignment() - 1);
	auto buffer_v = (std::byte*)::VirtualAlloc(nullptr, length_v, MEM_COMMIT | MEM_RESERVE, flags_v);

	if (nullptr==buffer_v) throw win32_error();

	m_buffer = buffer_v;
	m_length = length_v;
	m_access = access_v;
}

core::virtual_alloc_buffer::~virtual_alloc_buffer()
{
	while (!m_unmaps.empty()) {
		m_unmaps.back() ();
		m_unmaps.pop_back();
	}

	if (m_buffer) ::VirtualFree(m_buffer, 0, MEM_RELEASE);

	m_buffer = nullptr;
	m_length = 0u;
	m_access = 0u;
}

auto core::virtual_alloc_buffer::attach_to(core::hypervisor& host_v, std::uint64_t base_v, std::uint64_t size_v) -> void
{
	size_v = size_v>0u?std::min(size_v, m_length):m_length;
	host_v.map_physical_memory(m_buffer, base_v, size_v, m_access);
	m_unmaps.emplace_back([&host_v, base_v, size_v] () {
		host_v.unmap_physical_memory(base_v, size_v);
	});
}

auto core::virtual_alloc_buffer::lock_mutable(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte> 
{
	if (size_v == 0) size_v = m_length;
	if (base_v >= m_length) throw std::invalid_argument("base_v is out of range.");
	if (base_v + size_v > m_length) throw std::invalid_argument("size_v is out of range.");
	if (!m_locked.try_lock()) throw std::logic_error("Memory is already locked.");
	return std::span<std::byte>(m_buffer + base_v, size_v);
}

auto core::virtual_alloc_buffer::lock_constant(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte const>
{
	if (size_v == 0) size_v = m_length;
	if (base_v >= m_length) throw std::invalid_argument("base_v is out of range.");
	if (base_v + size_v > m_length) throw std::invalid_argument("size_v is out of range.");
	if (!m_locked.try_lock()) throw std::logic_error("Memory is already locked.");
	return std::span<std::byte const>(m_buffer + base_v, size_v);
}

auto core::virtual_alloc_buffer::unlock() -> void
{
	m_locked.unlock();
}

auto core::virtual_alloc_buffer::size() const -> std::uint64_t
{
	return m_length;
}
