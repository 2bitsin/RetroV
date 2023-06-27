#include <core/memory/manager.hpp>
#include <core/hypervisor.hpp>
#include <utils/span.hpp>
#include "manager.hpp"

using core::memory::Manager;

Manager::Manager(core::Hypervisor& hypervisor_v)
	: m_Hypervisor(hypervisor_v) 
{}

auto Manager::MapPhysical(std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t flags_v, std::uint64_t offset_v) -> void
{
	auto block_s = m_Hypervisor.GetMemoryPool().GetBlockView(index_v);
	block_s = block_s.subspan(offset_v, size_v > 0u ? size_v : block_s.size());
	auto partition_v = m_Hypervisor.GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvMapGpaRange(partition_v, block_s.data(), base_v, block_s.size(), (WHV_MAP_GPA_RANGE_FLAGS)flags_v));
}

void Manager::UnmapPhysical(std::uint64_t base_v, std::uint64_t size_v)
{
	auto partition_v = m_Hypervisor.GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvUnmapGpaRange(partition_v, base_v, size_v));
}

auto Manager::VirtualToPhysical(std::uint32_t index_v, std::uint64_t& inout_address_v,
	WHV_TRANSLATE_GVA_FLAGS flags_v) const -> WHV_TRANSLATE_GVA_RESULT_CODE
{
	WHV_TRANSLATE_GVA_RESULT result_v{ };
	auto const control0_v = m_Hypervisor.GetRegister<std::uint64_t>(index_v, WHvX64RegisterCr0);
	static constexpr const std::uint64_t kPagingEnabled = 0x80000000u;
	if (!(control0_v & kPagingEnabled)) {
		return WHvTranslateGvaResultSuccess;
	}
	auto partition_v = m_Hypervisor.GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvTranslateGva(partition_v, index_v,
		inout_address_v, flags_v, &result_v, &inout_address_v));
	return result_v.ResultCode;
}

auto Manager::Write(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> data_v, std::uint32_t flags_v, WHV_CACHE_TYPE chache_v) -> void
{
	while (!data_v.empty()) 
	{
		static constexpr auto M = kPageSize - 1u;
		auto const next_page_v = (address_v + M) & ~M;
		auto const ammount_v = std::min<std::uint64_t>(kMaxGpaReadWriteSize, next_page_v - address_v);
		auto next_bits_v = utils::take_span(data_v, ammount_v);
		WriteSome(index_v, address_v, next_bits_v, flags_v, chache_v);
		address_v += next_bits_v.size();
	}
}

auto Manager::Fetch(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte> data_v, std::uint32_t flags_v, WHV_CACHE_TYPE chache_v) -> void
{
	while (!data_v.empty())
	{
		static constexpr auto M = kPageSize - 1u;
		auto const next_page_v = (address_v + M) & ~M;
		auto const ammount_v = std::min<std::uint64_t>(kMaxGpaReadWriteSize, next_page_v - address_v);
		auto next_bits_v = utils::take_span(data_v, ammount_v);
		FetchSome(index_v, address_v, next_bits_v, flags_v, chache_v);
		address_v += next_bits_v.size();
	}
}

auto Manager::WriteSome(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> data_v,
	std::uint32_t flags_v, WHV_CACHE_TYPE chache_v) -> void
{
	if (flags_v&kValidatedAddress) {
		auto const v2pflags_v = (flags_v&kValidatedAddress 
			? WHvTranslateGvaFlagValidateWrite : WHvTranslateGvaFlagNone);
		if (WHvTranslateGvaResultSuccess!=VirtualToPhysical(index_v, address_v, v2pflags_v)) {
			throw std::runtime_error("Virtual address is not writable");
		}		
	}

	auto partition_v = m_Hypervisor.GetParitionHandle();
	WHV_ACCESS_GPA_CONTROLS const access_v{ .CacheType = chache_v };
	WIN32_ERROR_ASSERT(::WHvWriteGpaRange(partition_v, index_v, address_v, 
		access_v, data_v.data(), data_v.size()));
}

auto Manager::FetchSome(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte> data_v, 
	std::uint32_t flags_v, WHV_CACHE_TYPE chache_v) -> void
{
	if (flags_v & kValidatedAddress) {
		auto const v2pflags_v = (flags_v & kValidatedAddress
			? WHvTranslateGvaFlagValidateRead : WHvTranslateGvaFlagNone);
		if (WHvTranslateGvaResultSuccess != VirtualToPhysical(index_v, address_v, v2pflags_v)) {
			throw std::runtime_error("Virtual address is not writable");
		}
	}

	auto partition_v = m_Hypervisor.GetParitionHandle();
	WHV_ACCESS_GPA_CONTROLS const access_v{ .CacheType = chache_v };
	WIN32_ERROR_ASSERT(::WHvReadGpaRange(partition_v, index_v, address_v,
		access_v, data_v.data(), data_v.size()));

}
