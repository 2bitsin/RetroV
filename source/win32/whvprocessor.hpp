#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <core/constants.hpp>
#include <utils/span.hpp>

#include <vector>
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
		
		auto GetIndex() const -> std::uint32_t;

		auto Run() const -> std::tuple<std::int32_t, WHV_RUN_VP_EXIT_CONTEXT>;
		auto Reset() const -> std::int32_t;
    auto Run(WHV_RUN_VP_EXIT_CONTEXT& exit_v) const -> std::int32_t;
		auto Cancel() const -> std::int32_t;

		auto GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t;
		auto SetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE value_v) const -> std::int32_t;
		auto GetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE& value_v) const -> std::int32_t;

		template <typename T = WHV_REGISTER_VALUE> 
		requires (sizeof(T) <= sizeof(WHV_REGISTER_VALUE) && std::is_trivial_v<T>)
		inline auto GetRegister(WHV_REGISTER_NAME rname_v) const -> T {
			WHV_REGISTER_VALUE value_v{};
			WIN32_ERROR_ASSERT(GetRegister(rname_v, value_v));
			return reinterpret_cast<T const&>(value_v);
		}

		auto TranslateGva(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const -> std::tuple<std::int32_t, WHV_TRANSLATE_GVA_RESULT_CODE, std::uint64_t>;
		auto MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> data_v, WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) const -> std::int32_t;
		auto RequestIRQ(WHV_INTERRUPT_CONTROL irq_v) -> std::int32_t;

		auto GetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::vector<std::byte>& buffer_v) const -> std::int32_t;
		auto SetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::span<std::byte const> buffer_v) const -> std::int32_t;

	private:
		win32::WHvPartition const& m_Partition;
		std::uint32_t m_VcpuIndex;
	};
}
