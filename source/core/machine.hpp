#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>

#include <utils/as_bytes.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>
#include <core/memory.hpp>
#include <core/config.hpp>
#include <core/iohandler.hpp>
#include <core/vchandler.hpp>
#include <core/registerfile.hpp>

namespace core
{ 
	struct Machine
	{
		/*********************************
		 * Constants and type definitions
		 *********************************/

		static inline constexpr auto kAccessWrite = 1u;
		static inline constexpr auto kAccessFetch = 2u;

		static inline constexpr auto kMemoryFlagsAll = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsROM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsRAM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsDevice = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagTrackDirtyPages;
		
		/*********************************
		 *  Constructors and destructors
		 *********************************/

		Machine(Machine const&) = delete;
		auto operator = (Machine const&)->Machine & = delete;
		Machine(Machine&&) = delete;
		auto operator = (Machine&&)->Machine & = delete;

		Machine (Config const&);
	  ~Machine ();
	
		/*********************************
		 *  Memory configuration methods
		 *********************************/

		template <typename... T>
		auto InitializeMemory(T&&...args_v) -> std::size_t {
			auto const index_v = m_Memories.size();
			m_Memories.emplace_back(
				std::forward<T>(args_v)...);
			return index_v;
		}
		auto MapMemory (std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v=0u, std::uint32_t flags_v=kMemoryFlagsRAM, std::size_t offset_v=0u) -> void;
		auto UnmapMemory (std::uint64_t base_v, std::uint64_t size_v=0u) -> void;
		auto TranslateVirtualAddress (std::uint32_t index_v, std::uint64_t& inout_address_v, WHV_TRANSLATE_GVA_FLAGS flags_v = WHvTranslateGvaFlagNone) const -> WHV_TRANSLATE_GVA_RESULT_CODE;

		auto ReadPhysical(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte> buffer_v, WHV_CACHE_TYPE cache_control_v=WHvCacheTypeUncached) const -> void;
		auto WritePhysical(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> buffer_v, WHV_CACHE_TYPE cache_control_v=WHvCacheTypeWriteThrough) const -> void;

		template <typename T> requires (std::is_trivially_copyable_v<T>)
		auto ReadPhysical(std::uint32_t index_v, std::uint64_t address_v, 
			WHV_CACHE_TYPE cache_control_v = WHvCacheTypeUncached) const -> T {
			T buffer_v { };
			ReadPhysical(index_v, address_v, utils::as_mutable_bytes(buffer_v), cache_control_v);
			return buffer_v;
		}	

		template <typename T> requires (std::is_trivially_copyable_v<T>)
		auto WritePhysical(std::uint32_t index_v, std::uint64_t address_v, T const& buffer_v,
			WHV_CACHE_TYPE cache_control_v = WHvCacheTypeUncached) const -> void {
			return WritePhysical(index_v, address_v, utils::as_bytes(buffer_v), cache_control_v);
		}
		
		/****************************
		 *  I/O configuration methods
		 ****************************/
		auto MapIoRange(IOHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v = kAccessFetch | kAccessWrite) -> void;
		auto UnmapIoRange(std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v = kAccessFetch | kAccessWrite) -> void;

		/*******************************
		 *  VMCALL configuration methods
		 *******************************/
		auto MapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void;
		auto UnmapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void;
		auto UnmapVcRange(std::uint16_t base_v, std::uint16_t size_v) -> void;

		/**********************************
		 *  Processor configuration methods
		 **********************************/
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
		auto GetRegisters(std::uint32_t index_v) const -> RegisterFile;
		auto SetRegisters(std::uint32_t index_v, RegisterFile const&) -> void;
		static auto IsVendorIntel() -> bool;
		static auto IsVendorAMD() -> bool;

		/*************************************
		 *  Utility stuff
		 *************************************/
		static auto GetCapability(WHV_CAPABILITY_CODE, void* buffer_v, std::uint32_t length_v) -> std::uint32_t;
		template <typename T>
		static inline auto GetCapability(WHV_CAPABILITY_CODE code_v) -> T {
			T buffer_v{ };
			[[maybe_unused]] auto const length_v = GetCapability(code_v, &buffer_v, sizeof(buffer_v));
			//if(length_v == sizeof(buffer_v));
			return buffer_v;
		}

	protected:
		friend struct Config;


		/*************************************
		 *  Misc internal methods
		 *************************************/
		auto InitializePartitionProperties() -> void;
		auto RunVirtualProcessor(std::uint32_t index_v, std::stop_token token_v) -> void;

		/**********************************
		 *  Exit handling methods
		 **********************************/
		auto HandleExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleIoOperation(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleHypercall(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;

		/***************************
		 *  Debuging support methods
		 ***************************/
    auto Disassemble(std::ostream& output_v, std::uint32_t index_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void;

		/**********************************
		 *  Partition configuration methods
		 **********************************/
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

		/**********************************
		 * Internal state
		 **********************************/
	private:
		WHV_PARTITION_HANDLE m_Partition{ nullptr };
		std::vector<Memory> m_Memories; 
		std::vector<std::uint32_t> m_Processors;
		std::vector<IOHandler*> m_IoWrite{ 0x10000u, nullptr };
		std::vector<IOHandler*> m_IoFetch{ 0x10000u, nullptr };
		std::vector<std::vector<VCHandler*>> m_VmmCall{ };
		std::stop_source m_ProcessorBreak;
	};
}