#pragma once

#include <win32/whvpartition.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <cstdint>
#include <cstddef>
#include <span>

namespace win32 
{
	struct VirtualMahcineBase;

	struct WHvProcessor
	{
		WHvProcessor(win32::WHvPartition& partition_v, std::uint32_t vcpuindex_v);
	  ~WHvProcessor() noexcept(false);

		auto Run() const -> std::tuple<HRESULT, WHV_RUN_VP_EXIT_CONTEXT>;
		auto Cancel() const -> HRESULT;

		auto GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> HRESULT;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> HRESULT;

		auto TranslateGva(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const -> std::tuple<HRESULT, WHV_TRANSLATE_GVA_RESULT, std::uint64_t>;

	private:
		win32::WHvPartition& m_Partition;
		std::uint32_t m_VcpuIndex;		
	};
}
