#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <core/constants.hpp>

#include <cstdint>
#include <cstddef>
#include <span>

namespace win32 
{
	struct WHvPartition;

	struct WHvProcessor
	{		
		WHvProcessor(WHvPartition const& partition_v, std::uint32_t vcpuindex_v=0u);
	  ~WHvProcessor() = default;

		auto Run() const -> std::tuple<HRESULT, WHV_RUN_VP_EXIT_CONTEXT>;
		auto Reset() const -> HRESULT;
    auto Run(WHV_RUN_VP_EXIT_CONTEXT& exit_v) const->HRESULT;
		auto Cancel() const -> HRESULT;

		auto GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> HRESULT;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> HRESULT;
		auto SetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE value_v) const -> HRESULT;
		auto GetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE& value_v) const -> HRESULT;
		auto TranslateGva(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const -> std::tuple<HRESULT, WHV_TRANSLATE_GVA_RESULT_CODE, std::uint64_t>;
		auto MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, std::uint8_t size_v, std::span<std::byte, 8u> data_v, WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) const -> std::tuple<HRESULT, std::size_t>;

	private:
		win32::WHvPartition const& m_Partition;
		std::uint32_t m_VcpuIndex;		
	};
}
