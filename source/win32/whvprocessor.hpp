#pragma once

#include <win32/whvregisters.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

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
		WHvProcessor(WHvPartition& partition_v, uint32_t vcpuindex_v=0u);
	  ~WHvProcessor() = default;
		
		auto GetIndex() const -> uint32_t;

		/**********************
		 *	PROCESSOR EXECUTION
		 **********************/
		auto RunToExit() const -> std::tuple<int32_t, WHV_RUN_VP_EXIT_CONTEXT>;
		auto Reset() const -> int32_t;
    auto RunToExit(WHV_RUN_VP_EXIT_CONTEXT& exit_v) const -> int32_t;
		auto Cancel() const -> int32_t;

		/************************
		 *	MISC STATE MANAGEMENT
		 ************************/
		auto GetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::vector<std::byte>& buffer_v) const->int32_t;
		auto SetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::span<std::byte const> buffer_v) const->int32_t;

		/**********************
		 *	REGISTER OPERATIONS
		 **********************/
		auto GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> int32_t;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> int32_t;
		auto SetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE value_v) const -> int32_t;
		auto GetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE& value_v) const -> int32_t;
		
		template <typename... Regs>
		inline auto GetRegisters(WHvRegisters<Regs...>& regs_v) const -> int32_t {
			return GetRegisters(regs_v.Names(), regs_v.Values());
		}

		template <typename... Regs>
		inline auto GetRegisters() const -> WHvRegisters<Regs...> {
			WHvRegisters<Regs...> regs_v{};
			WIN32_ERROR_ASSERT(GetRegisters(regs_v));
			return regs_v;
		}

		template <typename... Regs>
		inline auto SetRegisters(WHvRegisters<Regs...> const& regs_v) const -> int32_t {
			return SetRegisters(regs_v.Names(), regs_v.Values());
		}

		template <typename... Regs>
		inline auto RegistersScoped() -> WHvRegistersScoped<WHvProcessor, Regs...> {
			return { *this };
		}

		template <typename T = WHV_REGISTER_VALUE> 
		requires (sizeof(T) <= sizeof(WHV_REGISTER_VALUE) && std::is_trivial_v<T>)
		inline auto GetRegister(WHV_REGISTER_NAME rname_v) const -> T {
			WHV_REGISTER_VALUE value_v{};
			WIN32_ERROR_ASSERT(GetRegister(rname_v, value_v));
			return reinterpret_cast<T const&>(value_v);
		}

		/***************************
		 *	ADDRESS SPACE OPERATIONS
		 ***************************/
		auto TranslateGva(uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const -> std::tuple<int32_t, WHV_TRANSLATE_GVA_RESULT_CODE, uint64_t>;
		auto MemoryAccess(bool is_write_v, uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v, WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) const -> int32_t;

		template<typename T> requires(std::is_trivial_v<T>)
		auto MemoryWrite(uint64_t physaddr_v, T what_v) const -> int32_t {
			return MemoryAccess(true, physaddr_v, utils::as_static_mutable_bytes(what_v));
		}

		template<typename T> requires(std::is_trivial_v<T>)
		auto MemoryFetch(uint64_t physaddr_v, T& what_v) const -> int32_t {
			return MemoryAccess(false, physaddr_v, utils::as_static_mutable_bytes(what_v));
		}

		template<typename T> requires(std::is_trivial_v<T>)
		auto MemoryFetch(uint64_t physaddr_v) const -> T {
			T what_v{};
			WIN32_ERROR_ASSERT(MemoryFetch(physaddr_v, what_v));
			return what_v;
		}

		/***********************
		 *	INTERRUPT MANAGEMENT
		 ***********************/
		auto RequestInterrupt(WHV_INTERRUPT_CONTROL irq_v) -> int32_t;


	private:
		win32::WHvPartition& m_Partition;
		uint32_t m_VcpuIndex;
	};
}
