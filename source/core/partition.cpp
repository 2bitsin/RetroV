#include <core/partition.hpp>
#include <core/hypervisor.hpp>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

using core::Partition;

Partition::Partition(Hypervisor& hypervisor_v)
	:	m_Hypervisor{ &hypervisor_v }
	,	m_Handle{ nullptr }
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Handle));
}

auto Partition::operator=(Partition&& prev_v) noexcept -> Partition& {
	if (this==&prev_v)
		return *this;
	auto temp_v{ std::move(prev_v) };
	temp_v.Swap(*this);
	return *this;
}

auto Partition::Swap(Partition& other_v) noexcept -> void
{
	std::swap(m_Hypervisor, other_v.m_Hypervisor);
	std::swap(m_Handle, other_v.m_Handle);
}

Partition::Partition(Partition&& prev_v) noexcept
:	m_Handle{ std::exchange(prev_v.m_Handle, nullptr) }
{}

Partition::~Partition()
{
	if (m_Handle) {
		::WHvDeletePartition(m_Handle);
		m_Handle = nullptr;
	}
}

auto Partition::GetHandle() const -> WHV_PARTITION_HANDLE
{
	return m_Handle;
}

auto Partition::Setup() const -> void
{
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_Handle));
}

auto Partition::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte const> buffer_v) const -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Handle, code_v, buffer_v.data(), buffer_v.size()));
}

auto Partition::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte>& buffer_v) const -> void
{
	std::uint32_t size_v{ 0 };
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Handle, code_v, buffer_v.data(), buffer_v.size(), &size_v));
	buffer_v = buffer_v.first(size_v);
}
