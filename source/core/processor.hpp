#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <core/processor/registers.hpp>
#include <core/hypervisor_fwd.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <future>
#include <thread>
#include <mutex>
#include <span>

namespace core
{	

	struct Processor
	{
		Processor(core::Hypervisor& hypervisor_v, std::uint32_t index_v);

		auto operator = (Processor const&)->Processor & = delete;
		Processor(Processor const&) = delete;

		auto operator = (Processor&&) noexcept -> Processor&;
		Processor(Processor&&) noexcept;

		auto Swap (Processor& other_v) noexcept -> void;

		~Processor();

		auto GetIndex() const -> std::uint32_t;		

		auto GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) const -> HRESULT;			                
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> HRESULT;

		auto GetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE& value_v) const -> void;
		auto SetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE const& value_v) const -> void;

		template <typename T>
		auto GetRegister(WHV_REGISTER_NAME name_v) const -> T 
		{
			T result_v { };
			WHV_REGISTER_VALUE value_v{ 0 };
			static constexpr const auto kSize = std::min(
				sizeof(result_v), sizeof(value_v));
			GetRegister(name_v, value_v);
			std::memcpy(&result_v, &value_v, kSize);
			return result_v;
		}

		template <typename T>
		auto SetRegister(WHV_REGISTER_NAME name_v, T const& value_v) const -> void
		{
			static constexpr const auto kSize = std::min(
				sizeof(value_v), sizeof(WHV_REGISTER_VALUE));
			WHV_REGISTER_VALUE value_s{ 0 };
			std::memcpy(&value_s, &value_v, kSize);
			SetRegister(name_v, value_s);
		}		

		auto GetRegisters() const -> RegisterFile;
		auto SetRegisters(RegisterFile const& registers_v) -> void;

		auto RunUntilExit () -> WHV_RUN_VP_EXIT_CONTEXT;		
		auto CancelRun() -> void;
		auto GetPartitionHandle() const -> WHV_PARTITION_HANDLE;

		auto RequestInterrupt(std::uint16_t vector_v, bool is_nmi_v = false) -> bool;

		auto IoPortAccess(WHV_EMULATOR_IO_ACCESS_INFO& access_v) -> bool;
		auto MemoryAccess(WHV_EMULATOR_MEMORY_ACCESS_INFO& access_v) -> bool;

	private:
		struct ProcThread{ std::jthread m_Thread; };
		Hypervisor* m_Hypervisor{ nullptr };
		std::uint32_t m_VProcIndex{ 0xffffffffu };

		std::unique_ptr<ProcThread> m_ProcThread;
	};

}