#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <core/hypervisor_fwd.hpp>
#include <core/processor.hpp>
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

		using Processor = Processor;

		using event_handler = std::function<bool(Processor&, WHV_RUN_VP_EXIT_CONTEXT const&)>;

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

		
		auto ConnectIoWrite(std::uint16_t base_v, std::uint16_t size_v, auto&& handler_v) -> void;
		auto ConnectIoFetch(std::uint16_t base_v, std::uint16_t size_v, auto&& handler_v) -> void;


	protected:
		auto DispatchIoAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;
		auto DispatchMemAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;
		auto DispatchHypercallEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;

	private:
		Hypervisor* m_Hypervisor{ nullptr };
		struct DispatchTables
		{
			std::shared_mutex m_Mutex;
			utils::interval_map<std::uint16_t, event_handler> m_IOWrite;
			utils::interval_map<std::uint16_t, event_handler> m_IOFetch;
			utils::interval_map<std::uint16_t, event_handler> m_Hypercall;
			utils::interval_map<std::uint64_t, event_handler> m_MemWrite;
			utils::interval_map<std::uint64_t, event_handler> m_MemFetch;
		};
		std::unique_ptr<DispatchTables> m_DispatchTbl;
	};


	template<typename T>
	inline auto EventBroker::ConnectIoWrite(std::uint16_t base_v, std::uint16_t size_v, T&& handler_v) -> void {
		return SetupRange(base_v, size_v, [handler_v = std::forward<T>(handler_v)](
			Processor& vcpu_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
				auto const& ioinfo_v{ exit_v.IoPortAccess };
				auto const& access_v{ ioinfo_v.AccessInfo };
				auto const port_v = ioinfo_v.PortNumber;
				auto const size_v = access_v.AccessSize;
				auto const data_v = ioinfo_v.Rax;
				return handler_v->IoWrite(vcpu_v, port_v, data_v, size_v);
			}, IOType::kIoWrite);
	}

	template<typename T>
	inline auto EventBroker::ConnectIoFetch(std::uint16_t base_v, std::uint16_t size_v, T&& handler_v) -> void {
		return SetupRange(base_v, size_v, [handler_v = std::forward<T>(handler_v)](
			Processor& vcpu_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
				auto const& ioinfo_v{ exit_v.IoPortAccess };
				auto const& access_v{ ioinfo_v.AccessInfo };
				std::uint64_t result_v{ 0 };
				auto const port_v = ioinfo_v.PortNumber;
				auto const size_v = access_v.AccessSize;
				auto return_v = handler_v->IoFetch(vcpu_v, port_v, result_v, size_v);
				vcpu_v.SetRegister(WHvX64RegisterRax, result_v);
				return return_v;
			}, IOType::kIoFetch);
	}

}
