#include <core/hypervisor.hpp>
#include <core/config.hpp>
#include <core/config_impl.hpp>
#include <core/machine_monitor.hpp>
#include <core/aligned_memory.hpp>
#include <core/registers.hpp>

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

static constexpr const WHV_REGISTER_NAME G_register_names[] = {
	/*  0 */ WHvX64RegisterRflags,
	/*  1 */ WHvX64RegisterRip,
	/*  2 */ WHvX64RegisterCs,
	/*  3 */ WHvX64RegisterDs,
	/*  4 */ WHvX64RegisterEs,
	/*  5 */ WHvX64RegisterSs,
	/*  6 */ WHvX64RegisterFs,
	/*  7 */ WHvX64RegisterGs,
	/*  8 */ WHvX64RegisterIdtr,
	/*  9 */ WHvX64RegisterGdtr,
	/*  A */ WHvX64RegisterRax,
	/*  B */ WHvX64RegisterRbx,
	/*  C */ WHvX64RegisterRcx,
	/*  D */ WHvX64RegisterRdx,
	/*  E */ WHvX64RegisterRsi,
	/*  F */ WHvX64RegisterRdi,
	/* 10 */ WHvX64RegisterRbp,
	/* 11 */ WHvX64RegisterRsp,
	/* 12 */ WHvX64RegisterR8,
	/* 13 */ WHvX64RegisterR9,
	/* 14 */ WHvX64RegisterR10,
	/* 15 */ WHvX64RegisterR11,
	/* 16 */ WHvX64RegisterR12,
	/* 17 */ WHvX64RegisterR13,
	/* 18 */ WHvX64RegisterR14,
	/* 19 */ WHvX64RegisterR15,
	/* 1A */ WHvX64RegisterCr0,
};

WHV_REGISTER_VALUE G_real_mode_values[] = {
	{.Reg64 = 0x0000000000000002u },
	{.Reg64 = 0x000000000000FFF0u },

	{.Segment = {.Base = 0xf0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Table = {.Limit = 0x03FFu, .Base = 0x00000000u  } },
	{.Table = {.Limit = 0x0000u, .Base = 0x00000000u  } },

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000010u}
};


struct whpx_hypervisor final: public core::hypervisor
{
	whpx_hypervisor(std::unique_ptr<core::machine_monitor> monitor_v);
  ~whpx_hypervisor();

	auto map_physical_memory(void*, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void override final;
	auto unmap_physical_memory(std::uint64_t base_v, std::uint64_t size_v) -> void override final;

	auto start_machine() -> void override final;


protected:
	auto setup_partition () -> void;
	auto run_virtual_processor (std::uint32_t index_v) -> bool; 
	auto handle_io_access(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& context_v) -> bool;
	auto set_instruction_pointer(std::uint32_t index_v, std::uint64_t target_v) -> void;
	auto get_registers(std::uint32_t index_v, x64_registers& registers_v) -> void;
	auto set_registers(std::uint32_t index_v, x64_registers const& registers_v) -> void;
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

auto whpx_hypervisor::run_virtual_processor(std::uint32_t index_v) -> bool
{
	WHV_RUN_VP_EXIT_CONTEXT exit_v;
	WIN32_ERROR_ASSERT(::WHvRunVirtualProcessor(m_partition, index_v, &exit_v, sizeof(exit_v)));
	switch (exit_v.ExitReason)
	{
	case WHvRunVpExitReasonX64IoPortAccess:
		return handle_io_access(index_v, exit_v);
	default:
		throw std::runtime_error("Unhandled exit reason");
	}
}



auto whpx_hypervisor::handle_io_access(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& context_v) -> bool
{
	
}

auto whpx_hypervisor::set_instruction_pointer(std::uint32_t index_v, std::uint64_t target_v) -> void
{	
	WHV_REGISTER_NAME name_v = WHvX64RegisterRip;
	WHV_REGISTER_VALUE value_v = { .Reg64 = target_v };
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_partition, index_v, &name_v, 1u, &value_v));
}

auto whpx_hypervisor::get_registers(std::uint32_t index_v, x64_registers& registers_v) -> void
{

}

auto whpx_hypervisor::set_registers(std::uint32_t index_v, x64_registers const& registers_v) -> void
{
	WHV_REGISTER_VALUE value_v[] = {
		{ .Reg64 = registers_v.rax },
		{ .Reg64 = registers_v.rbx },
		{ .Reg64 = registers_v.rcx },
		{ .Reg64 = registers_v.rdx },
		{ .Reg64 = registers_v.rsi },
		{ .Reg64 = registers_v.rdi },
		{ .Reg64 = registers_v.rbp },
		{ .Reg64 = registers_v.rsp },
		{ .Reg64 = registers_v.r8 },
		{ .Reg64 = registers_v.r9 },
		{ .Reg64 = registers_v.r10 },
		{ .Reg64 = registers_v.r11 },
		{ .Reg64 = registers_v.r12 },
		{ .Reg64 = registers_v.r13 },
		{ .Reg64 = registers_v.r14 },
		{ .Reg64 = registers_v.r15 },
		{ .Reg64 = registers_v.rip },
		{ .Reg64 = registers_v.rflags },

	#define S(X) \
		{ .Segment = {\
			.Base = registers_v.X.base,\
			.Limit = registers_v.X.size,\
			.Selector = registers_v.X.value,\
			.Attributes = registers_v.X.attr}}
		
		S(cs),
		S(ds),
		S(es),
		S(fs),
		S(gs),
		S(ss),		
		S(ldtr),

		#undef S
		{ .Table = {.Limit = registers_v.gdtr.limit, .Base = registers_v.gdtr.base } },
		{ .Table = {.Limit = registers_v.idtr.limit, .Base = registers_v.idtr.base } },

		{ .Reg64 = registers_v.cr0 },
		{ .Reg64 = registers_v.cr2 },
		{ .Reg64 = registers_v.cr3 },
		{ .Reg64 = registers_v.cr4 },
		{ .Reg64 = registers_v.cr8 },
		{ .Reg64 = registers_v.dr0 },
		{ .Reg64 = registers_v.dr1 },
		{ .Reg64 = registers_v.dr2 },
		{ .Reg64 = registers_v.dr3 },
		{ .Reg64 = registers_v.dr6 },
		{ .Reg64 = registers_v.dr7 },


	};
}

auto core::hypervisor::create(std::unique_ptr<machine_monitor> monitor_v) -> std::unique_ptr<hypervisor>
{
	return std::make_unique<whpx_hypervisor>(std::move(monitor_v));	
}
 

