#include <core/hypervisor.hpp>
#include <core/config.hpp>
#include <core/config_impl.hpp>
#include <core/machine_monitor.hpp>
#include <core/aligned_memory.hpp>

#include <win32/windows.hpp>
#include <win32/win32_error.hpp>

#include <stop_token>
#include <thread>
#include <future>
#include <vector>
#include <memory>
#include <mutex>
#include <span>

#include <WinHvPlatform.h>
#include <WinHvPlatformDefs.h>

struct whpx_hypervisor final: public core::hypervisor
{
	whpx_hypervisor(std::unique_ptr<core::machine_monitor> monitor_v);
  ~whpx_hypervisor();

	auto map_physical_memory(void*, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void override final;
	auto unmap_physical_memory(std::uint64_t base_v, std::uint64_t size_v) -> void override final;

	auto start_machine() -> void override final;


protected:
	auto setup_partition () -> void;
	auto run_virtual_processor (std::uint32_t index_v) -> void; 
	auto handle_io_access(std::uint32_t index_v, WHV_X64_IO_PORT_ACCESS_CONTEXT& context_v) -> void;
	
private:
	std::unique_ptr<core::machine_monitor> m_vmmonitor;
	std::vector<std::unique_ptr<core::aligned_memory>> m_memory;
	std::vector<std::uint32_t> m_cpus;
	std::vector<std::tuple<std::uint32_t, std::stop_source, std::future<void>>> m_threads;
	WHV_PARTITION_HANDLE m_partition;
};

whpx_hypervisor::whpx_hypervisor(std::unique_ptr<core::machine_monitor> monitor_v)
	: m_vmmonitor(std::move(monitor_v))
	, m_partition(nullptr)
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_partition));
	setup_partition();
}

whpx_hypervisor::~whpx_hypervisor()
{
	if (nullptr==m_partition) return;
	::WHvDeletePartition(m_partition);
}

auto whpx_hypervisor::map_physical_memory(void* source_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void
{
	std::uint32_t flags_v { 0u };
	if (access_v & core::access::read)
		flags_v |= WHvMemoryAccessRead;
	if (access_v & core::access::write)
		flags_v |= WHvMemoryAccessWrite;
	if (access_v & core::access::execute)
		flags_v |= WHvMemoryAccessExecute;
	WIN32_ERROR_ASSERT(::WHvMapGpaRange(m_partition, source_v, base_v, size_v, 
		(WHV_MAP_GPA_RANGE_FLAGS)flags_v));
}

auto whpx_hypervisor::unmap_physical_memory(std::uint64_t base_v, std::uint64_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvUnmapGpaRange(m_partition, base_v, size_v));
}

auto whpx_hypervisor::start_machine() -> void
{
	if (m_cpus.empty()) throw std::logic_error("No processors configured");
	for (auto&& index_v : m_cpus) 
	{
		std::stop_source stop_source_v;
		std::stop_token stop_token_v = stop_source_v.get_token();
		m_threads.emplace_back(index_v, std::move(stop_source_v), 
			std::async(std::launch::async,
				[this] (auto index_v, auto stop_token_v) -> void 
				{	while (!stop_token_v.stop_requested()) 
						run_virtual_processor(index_v); },
				index_v, std::move(stop_token_v)));
	}

	for (auto&[index_v, stop_source_v, future_v]: m_threads) {		
		future_v.wait();
	}
}

auto whpx_hypervisor::setup_partition() -> void
{
	auto config_v = core::config::create();
	m_vmmonitor->initialize(*config_v);
	auto& config_impl_v = static_cast<core::config_impl&>(*config_v);
	std::vector<std::uint32_t> cpus_v;
	for (auto&& item_v : config_impl_v.items()) {
		std::visit([this, &cpus_v]<typename T>(T const& what_v) 
		{
			if constexpr (std::is_same_v<T, core::config_impl::processor_item>) {
				cpus_v.push_back(what_v.index);
			} 
			else if constexpr (std::is_same_v<T, core::config_impl::memory_from_file_item>) {
				auto memory_v = core::aligned_memory::from_file(what_v.source, what_v.access);
				memory_v->attach_to(*this, what_v.base, what_v.size);
				m_memory.emplace_back(std::move (memory_v));				
			} 
			else if constexpr (std::is_same_v<T, core::config_impl::memory_from_bytes_item>) {
				auto memory_v = core::aligned_memory::from_bytes(what_v.source, what_v.access);
				memory_v->attach_to(*this, what_v.base, what_v.size);
				m_memory.emplace_back(std::move(memory_v));
			}
		}, item_v);
	}
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_partition));
	for (auto&& index_v : cpus_v) {
		WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_partition, index_v, 0));
		m_cpus.push_back(index_v);
	}
}

auto whpx_hypervisor::run_virtual_processor(std::uint32_t index_v) -> void
{
	WHV_RUN_VP_EXIT_CONTEXT exit_v;
	WIN32_ERROR_ASSERT(::WHvRunVirtualProcessor(m_partition, index_v, &exit_v, sizeof(exit_v)));
	switch (exit_v.ExitReason)
	{
	case WHvRunVpExitReasonX64IoPortAccess:
		return handle_io_access(index_v, exit_v.IoPortAccess);	
	default:
		throw std::runtime_error("Unhandled exit reason");
	}
}

auto whpx_hypervisor::handle_io_access(std::uint32_t index_v, WHV_X64_IO_PORT_ACCESS_CONTEXT& context_v) -> void
{
}

auto core::hypervisor::create(std::unique_ptr<machine_monitor> monitor_v) -> std::unique_ptr<hypervisor>
{
	return std::make_unique<whpx_hypervisor>(std::move(monitor_v));	
}
 