#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <core/hypervisor_fwd.hpp>
#include <core/cpu/processor_fwd.hpp>
#include <utils/enums.hpp>
#include <utils/interval_map.hpp>

#include <shared_mutex>
#include <functional>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <array>

namespace core
{

	struct EventBroker
	{
		enum IOType : std::uint32_t {
			kIoFetch = 0x01u,
			kIoWrite = 0x02u,
			kMemFetch = 0x04u,
			kMemWrite = 0x08u,
			kVMCall = 0x10u
		};

		HVDOS_DEFINE_ENUM_FLAG_OPERATORS(IOType);

		using Processor = cpu::Processor;

		using event_handler = std::function<void(Processor&, WHV_RUN_VP_EXIT_CONTEXT const&)>;

		EventBroker(Hypervisor& hypervisor_v);

		auto operator = (EventBroker const&) -> EventBroker & = delete;
		EventBroker(EventBroker const&) = delete;

		auto operator = (EventBroker&&) noexcept -> EventBroker&;
		EventBroker(EventBroker&&) noexcept;

		auto Swap(EventBroker& other_v) noexcept -> void;

		~EventBroker();

		auto DispatchEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;

		auto SetupRange(std::uint64_t base_v, std::size_t size_v, event_handler handler_v, IOType iotype_v = IOType::kIoFetch|IOType::kIoWrite) -> void;
		auto ClearRange(std::uint64_t base_v, std::size_t size_v, IOType iotype_v = IOType::kIoFetch|IOType::kIoWrite) -> void;

	protected:
		auto DispatchIoAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;
		auto DispatchMemAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;
		auto DispatchVMMCallEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;

	private:
		Hypervisor* m_Hypervisor{ nullptr };
		struct DispatchTables
		{
			std::shared_mutex m_Mutex;
			utils::interval_map<std::uint16_t, event_handler> m_IOWrite;
			utils::interval_map<std::uint16_t, event_handler> m_IOFetch;
			utils::interval_map<std::uint16_t, event_handler> m_VMMCall;
			utils::interval_map<std::uint64_t, event_handler> m_Memory;
		};
		std::unique_ptr<DispatchTables> m_DiaptchTbl;
	};


}
