#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <any>

#include <utils/as_bytes.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>
#include <core/memory.hpp>
#include <core/config.hpp>
#include <core/iodevice.hpp>

namespace core
{ 
	struct Machine
	{
		static inline constexpr auto kAccessWrite = 1u;
		static inline constexpr auto kAccessFetch = 2u;

		static inline constexpr auto kMemoryFlagsAll = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsROM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsRAM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsDevice = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagTrackDirtyPages;
		
	  Machine (Config const&);
	  ~Machine ();
	
		Machine (Machine const&) = delete;
		auto operator = (Machine const&) -> Machine& = delete;
		Machine (Machine &&) = delete;
		auto operator = (Machine &&) -> Machine& = delete;
	
		auto MapMemory (std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v=0u, std::uint32_t flags_v=kMemoryFlagsRAM, std::size_t offset_v=0u) -> void;	
		auto UnmapMemory (std::uint64_t base_v, std::uint64_t size_v=0u) -> void;
		auto TranslateVirtualAddress (std::uint32_t index_v, std::uint64_t& inout_address_v, WHV_TRANSLATE_GVA_FLAGS flags_v = WHvTranslateGvaFlagNone) const -> WHV_TRANSLATE_GVA_RESULT_CODE;
		auto ReadPhysical(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte> buffer_v, WHV_CACHE_TYPE cache_control_v=WHvCacheTypeUncached) const -> void;
		template <typename T> requires (std::is_trivially_copyable_v<T>)
			auto ReadPhysical(std::uint32_t index_v, std::uint64_t address_v, 
				WHV_CACHE_TYPE cache_control_v = WHvCacheTypeUncached) const -> T {
			T buffer_v { };
			ReadPhysical(index_v, address_v, utils::as_mutable_bytes(buffer_v), cache_control_v);
			return buffer_v;
		}
	
		template <typename... T>
		auto InitializeMemory(T&&...args_v) -> std::size_t {
			auto const index_v = m_Memories.size();
			m_Memories.emplace_back(
				std::forward<T>(args_v)...);
			return index_v;
		}
		
		auto MapIoRange(IODevice& device_v, std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v = kAccessFetch | kAccessWrite) -> void;
		auto UnmapIoRange(std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v = kAccessFetch | kAccessWrite) -> void;
		auto InitializeProcessor(std::uint32_t index) -> void;
		auto Run() -> void;

		template <typename T>
		auto SetRegister(std::uint32_t index_v, WHV_REGISTER_NAME name_v, T const& value_v) -> void {
			WHV_REGISTER_VALUE value_s { 0 };
			std::memcpy(&value_s, &value_v, std::min(sizeof(value_s), sizeof(value_v)));
			WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v, &name_v, 1u, &value_s));
		}

		template <typename T>
		auto GetRegister(std::uint32_t index_v, WHV_REGISTER_NAME name_v) const -> T {
			WHV_REGISTER_VALUE value_s { 0 };
			WIN32_ERROR_ASSERT(::WHvGetVirtualProcessorRegisters(m_Partition, index_v, &name_v, 1u, &value_s));
			T value_v { 0 };
			std::memcpy(&value_v, &value_s, std::min(sizeof(value_s), sizeof(value_v)));
			return value_v;
		}

		static auto IsVendorIntel() -> bool;
		static auto IsVendorAMD() -> bool;
	protected:
		friend struct Config;

		static auto GetCapability(WHV_CAPABILITY_CODE, void* buffer_v, std::uint32_t length_v) -> std::uint32_t;

		template <typename T>
		static inline auto GetCapability(WHV_CAPABILITY_CODE code_v) -> T {
			T buffer_v { };
			[[maybe_unused]] auto const length_v = GetCapability(code_v, &buffer_v, sizeof(buffer_v));
			//if(length_v == sizeof(buffer_v));
			return buffer_v;
		}

		auto InitializePartitionProperties() -> void;
		auto RunVirtualProcessor(std::uint32_t index_v, std::stop_token token_v) -> void;

		auto HandleExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleIoOperation(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleHypercall(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;

		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void;

		template <typename T>
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T const& data_v) -> void {
			static_assert(std::is_trivially_copyable_v<T>);
			return SetProperty(code_v, &data_v, sizeof(T));
		}

		template <typename T>
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T& data_v) -> void {
			static_assert(std::is_trivially_copyable_v<T>);
			auto size_v = sizeof(T);
			return GetProperty(code_v, &data_v, size_v);
		}

	private:
		WHV_PARTITION_HANDLE m_Partition{ nullptr };
		std::vector<Memory> m_Memories; 
		std::vector<std::uint32_t> m_Processors;
		std::vector<IODevice*> m_IoWrite{ 0x10000u, nullptr };
		std::vector<IODevice*> m_IoFetch{ 0x10000u, nullptr };
		std::vector<VMCallDevice*> m_VmCall{ 0x10000u, nullptr };
		std::stop_source m_ProcessorBreak;
	};
}