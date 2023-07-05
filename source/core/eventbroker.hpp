#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <core/hypervisor_fwd.hpp>
#include <utils/enums.hpp>

#include <functional>
#include <cstdint>
#include <cstddef>



namespace core
{

	struct EventBroker
	{
		using Processor = cpu::Processor;

		enum IOType : std::uint32_t {
			kIoFetch = 0x01u,
			kIoWrite = 0x02u,
			kMemFetch = 0x04u,
			kMemWrite = 0x08u,
			kVMCall = 0x10u
		};

		DEFINE_ENUM_FLAG_OPERATORS(IOType);

		using exit_handler = std::function<void(Processor&, WHV_RUN_VP_EXIT_CONTEXT const&)>;

		EventBroker(Hypervisor& hypervisor_v);

		auto operator = (EventBroker const&) -> EventBroker & = delete;
		EventBroker(EventBroker const&) = delete;

		auto operator = (EventBroker&&) noexcept -> EventBroker&;
		EventBroker(EventBroker&&) noexcept;

		auto Swap(EventBroker& other_v) noexcept -> void;

		~EventBroker();

		void DispatchEvent(Processor& processor_v, WHV_RUN_VP_EXIT_REASON const& exit_v);

		void InstallHandler(std::uint64_t base_v, std::size_t size_v, exit_handler handler_v, IOType iotype_v = IOType::kIoFetchAndWrite);
		void RemoveHandler(std::uint64_t base_v, std::size_t size_v, IOType iotype_v = IOType::kIoFetchAndWrite);

	private:
		Hypervisor* m_Hypervisor{ nullptr };

		struct DispatchTable {
			std::array<0x10000u, exit_handler> m_IoFetch;
			std::array<0x10000u, exit_handler> m_IoWrite;
			std::array<0x10000u, exit_handler> m_VMCall;
		};

		std::unique_ptr<DispatchTable> m_DispatchTable;
	};
}
