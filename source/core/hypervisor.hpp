#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <mutex>

#include <utils/as_bytes.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>
#include <core/memory/block.hpp>
#include <core/memory/pool.hpp>
#include <core/memory/manager.hpp>
#include <core/io/manager.hpp>
#include <core/config.hpp>
#include <core/vchandler.hpp>
#include <core/registerfile.hpp>

namespace core
{ 
	struct Hypervisor
	{
		/*********************************
		 * Constants and type definitions
		 *********************************/

		static inline constexpr auto kAccessWrite = 1u;
		static inline constexpr auto kAccessFetch = 2u;
		
		/*********************************
		 *  Constructors and destructors
		 *********************************/

		Hypervisor(Hypervisor const&) = delete;
		auto operator = (Hypervisor const&)->Hypervisor & = delete;
		Hypervisor(Hypervisor&&) = delete;
		auto operator = (Hypervisor&&)->Hypervisor & = delete;

		Hypervisor (Config const&);
	  ~Hypervisor ();

		auto GetMemoryPool() -> memory::Pool&;
		auto GetParitionHandle() -> WHV_PARTITION_HANDLE;
		auto GetCpuIndexes() -> std::span<std::uint32_t const>;
		auto GetMemoryManager() -> memory::Manager&;
		auto GetIoManager() -> io::Manager&;
	
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
		auto HandleHypercall(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;
		auto HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool;

		/***************************
		 *  Debuging support methods
		 ***************************/
    auto PrintRegisters(std::ostream& output_v, core::RegisterFile const& R) -> void;
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
		memory::Pool m_MemoryPool;
		memory::Manager m_MemoryManager;
		io::Manager m_IoManager;
		WHV_PARTITION_HANDLE m_Partition{ nullptr };
		std::vector<std::uint32_t> m_Processors;
		std::vector<std::vector<VCHandler*>> m_VmmCall{ };
		std::stop_source m_ProcessorBreak;
	};
}