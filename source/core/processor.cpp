#include <core/processor.hpp>
#include <core/hypervisor.hpp>
#include <core/capabilities.hpp>

#include <stdexcept>
#include <future>
#include <thread>
#include <mutex>

struct InitialProcessorState
{
	static constexpr const WHV_REGISTER_NAME Names[] = {
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
		/* 1B */ WHvX64RegisterCr2,
		/* 1C */ WHvX64RegisterCr3,
		/* 1D */ WHvX64RegisterCr4,
	};

	static constexpr const WHV_REGISTER_VALUE Values[] =
	{
		/* FLAGS */ {.Reg64 = 0x0000000000000002u + 0x0200u },
		/* RIP   */ {.Reg64 = 0x000000000000FFF0u },
		/* CS    */ {.Segment = {.Base = 0xf0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } },
		/* DS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* ES    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* SS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* FS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* GS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* GDTR  */ {.Table = {.Limit = 0x03FFu, .Base = 0x00000000u  } },
		/* IDTR  */ {.Table = {.Limit = 0x0000u, .Base = 0x00000000u  } },
		/* RAX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RBX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RCX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RDX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RSI   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RDI   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RSP   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RBP   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R8    */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R9    */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R10   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R11   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R12   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R13   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R14   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R15   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR0   */ {.Reg64 = 0x0000'0000'6000'0010u},
		/* CR2   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR3   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR4   */ {.Reg64 = 0x0000'0000'0000'0000u}
	};

	static constexpr const std::size_t Count = std::min(std::size(Names), std::size(Values));
};

using core::Processor;

Processor::Processor(core::Hypervisor& hypervisor_v, std::uint32_t index_v)
	: m_Hypervisor{ &hypervisor_v }
	, m_VProcIndex{ index_v }
	, m_ProcThread{ nullptr }
{
	using S = InitialProcessorState;
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(handle_v, m_VProcIndex, 0));	
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(handle_v, m_VProcIndex, 
		S::Names, S::Count, S::Values));	
}

auto Processor::operator=(Processor&& prev_v) noexcept -> Processor& {
	if (this == &prev_v)
		return *this;	
	Processor temp_v{ std::move(prev_v) };
	temp_v.Swap(*this);
	return *this;
}

Processor::Processor(Processor&& prev_v) noexcept
	: m_Hypervisor{ prev_v.m_Hypervisor }
	, m_VProcIndex{ std::exchange(prev_v.m_VProcIndex, 0xffffffffu) }
	, m_ProcThread{ std::move(prev_v.m_ProcThread) }
{}

auto Processor::Swap(Processor& other_v) noexcept -> void {	
	std::swap(m_Hypervisor, other_v.m_Hypervisor);
	std::swap(m_VProcIndex, other_v.m_VProcIndex);
	std::swap(m_ProcThread, other_v.m_ProcThread);
}

auto Processor::RunInThread(core::EventBroker& broker_v) -> void {
	m_ProcThread = std::make_unique<ProcThread>();
	auto& context_v { *m_ProcThread };
	context_v.m_Thread = std::jthread([this, &broker_v](std::stop_token stop_it) -> void {
		std::atomic<bool> break_it{ false };
		std::stop_callback really_stop_it (stop_it, [this, &break_it] {
			break_it.store(true);
			CancelRun();
		});

		while (!stop_it.stop_requested()) {
			auto const exit_v = RunUntilExit();
			if (exit_v.ExitReason == WHvRunVpExitReasonCanceled) {
				if (break_it.load()) 
					break;
				continue;
			}
			if (broker_v.DispatchEvent(*this, exit_v)) {
				SetRegister(WHvX64RegisterRip, exit_v.VpContext.Rip +
					exit_v.VpContext.InstructionLength);
			} 	
	}});
}

Processor::~Processor() {
	if (m_VProcIndex != 0xffffffffu) {
		m_ProcThread.reset();
		::WHvDeleteVirtualProcessor((*m_Hypervisor).GetParitionHandle(), m_VProcIndex);
		m_VProcIndex = 0xffffffffu;
	}
}

auto Processor::GetIndex() const -> std::uint32_t {
	return m_VProcIndex;
}

auto Processor::GetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE& value_v) const -> void {
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvGetVirtualProcessorRegisters(handle_v, m_VProcIndex, &name_v, 1u, &value_v));
}

auto Processor::SetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE const& value_v) const -> void {
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(handle_v, m_VProcIndex, &name_v, 1u, &value_v));
}

auto Processor::GetRegisters() const -> RegisterFile {
	std::vector<WHV_REGISTER_VALUE> values_v;
	values_v.resize(std::size(RegisterFile::Layout));
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvGetVirtualProcessorRegisters(handle_v, m_VProcIndex,
		RegisterFile::Layout, std::size(RegisterFile::Layout), values_v.data()));
	return RegisterFile
	{
		.rax = values_v[0].Reg64,
		.rbx = values_v[1].Reg64,
		.rcx = values_v[2].Reg64,
		.rdx = values_v[3].Reg64,
		.rsi = values_v[4].Reg64,
		.rdi = values_v[5].Reg64,
		.rbp = values_v[6].Reg64,
		.rsp = values_v[7].Reg64,
		.r8 = values_v[8].Reg64,
		.r9 = values_v[9].Reg64,
		.r10 = values_v[10].Reg64,
		.r11 = values_v[11].Reg64,
		.r12 = values_v[12].Reg64,
		.r13 = values_v[13].Reg64,
		.r14 = values_v[14].Reg64,
		.r15 = values_v[15].Reg64,
		.rip = values_v[16].Reg64,
		.rflags = values_v[17].Reg64,

		.cs_base = values_v[18].Segment.Base,
		.cs_size = values_v[18].Segment.Limit,
		.cs = values_v[18].Segment.Selector,
		.cs_attr = values_v[18].Segment.Attributes,

		.ds_base = values_v[19].Segment.Base,
		.ds_size = values_v[19].Segment.Limit,
		.ds = values_v[19].Segment.Selector,
		.ds_attr = values_v[19].Segment.Attributes,

		.es_base = values_v[20].Segment.Base,
		.es_size = values_v[20].Segment.Limit,
		.es = values_v[20].Segment.Selector,
		.es_attr = values_v[20].Segment.Attributes,

		.fs_base = values_v[21].Segment.Base,
		.fs_size = values_v[21].Segment.Limit,
		.fs = values_v[21].Segment.Selector,
		.fs_attr = values_v[21].Segment.Attributes,

		.gs_base = values_v[22].Segment.Base,
		.gs_size = values_v[22].Segment.Limit,
		.gs = values_v[22].Segment.Selector,
		.gs_attr = values_v[22].Segment.Attributes,

		.ss_base = values_v[23].Segment.Base,
		.ss_size = values_v[23].Segment.Limit,
		.ss = values_v[23].Segment.Selector,
		.ss_attr = values_v[23].Segment.Attributes
	};
}

auto Processor::SetRegisters(RegisterFile const& registers_v) -> void {
	std::vector<WHV_REGISTER_VALUE> values_v;
	values_v.resize(std::size(RegisterFile::Layout));
	values_v[0].Reg64 = registers_v.rax;
	values_v[1].Reg64 = registers_v.rbx;
	values_v[2].Reg64 = registers_v.rcx;
	values_v[3].Reg64 = registers_v.rdx;
	values_v[4].Reg64 = registers_v.rsi;
	values_v[5].Reg64 = registers_v.rdi;
	values_v[6].Reg64 = registers_v.rbp;
	values_v[7].Reg64 = registers_v.rsp;
	values_v[8].Reg64 = registers_v.r8;
	values_v[9].Reg64 = registers_v.r9;
	values_v[10].Reg64 = registers_v.r10;
	values_v[11].Reg64 = registers_v.r11;
	values_v[12].Reg64 = registers_v.r12;
	values_v[13].Reg64 = registers_v.r13;
	values_v[14].Reg64 = registers_v.r14;
	values_v[15].Reg64 = registers_v.r15;
	values_v[16].Reg64 = registers_v.rip;
	values_v[17].Reg64 = registers_v.rflags;
	values_v[18].Segment = { .Base = registers_v.cs_base, .Limit = registers_v.cs_size, .Selector = registers_v.cs, .Attributes = registers_v.cs_attr };
	values_v[19].Segment = { .Base = registers_v.ds_base, .Limit = registers_v.ds_size, .Selector = registers_v.ds, .Attributes = registers_v.ds_attr };
	values_v[20].Segment = { .Base = registers_v.es_base, .Limit = registers_v.es_size, .Selector = registers_v.es, .Attributes = registers_v.es_attr };
	values_v[21].Segment = { .Base = registers_v.fs_base, .Limit = registers_v.fs_size, .Selector = registers_v.fs, .Attributes = registers_v.fs_attr };
	values_v[22].Segment = { .Base = registers_v.gs_base, .Limit = registers_v.gs_size, .Selector = registers_v.gs, .Attributes = registers_v.gs_attr };
	values_v[23].Segment = { .Base = registers_v.ss_base, .Limit = registers_v.ss_size, .Selector = registers_v.ss, .Attributes = registers_v.ss_attr };
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(handle_v, m_VProcIndex,
		RegisterFile::Layout, std::size(RegisterFile::Layout), values_v.data()));
}

auto Processor::RunUntilExit() -> WHV_RUN_VP_EXIT_CONTEXT {
	WHV_RUN_VP_EXIT_CONTEXT exit_v;
	std::memset(&exit_v, 0, sizeof(exit_v));
	auto handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvRunVirtualProcessor(handle_v, m_VProcIndex, &exit_v, sizeof(exit_v)));
	return exit_v;
}

auto Processor::CancelRun() -> void
{
	auto const handle_v = (*m_Hypervisor).GetParitionHandle();
	WIN32_ERROR_ASSERT(::WHvCancelRunVirtualProcessor(handle_v, m_VProcIndex, 0u));
}

auto Processor::RequestInterrupt(std::uint16_t vector_v, bool is_nmi_v) -> bool
{
	auto const handle_v = (*m_Hypervisor).GetParitionHandle();
	WHV_REGISTER_VALUE value_v;
	std::memset(&value_v, 0, sizeof(value_v));
	value_v.PendingInterruption.InterruptionPending = 1;
	value_v.PendingInterruption.InterruptionType = !is_nmi_v ? WHvX64PendingInterrupt : WHvX64PendingNmi;
	value_v.PendingInterruption.DeliverErrorCode = 0;
	value_v.PendingInterruption.InterruptionVector = vector_v;
	WHV_REGISTER_NAME const pending_name_v = WHvRegisterPendingInterruption;
	return S_OK == ::WHvSetVirtualProcessorRegisters(handle_v, m_VProcIndex, &pending_name_v, 1u, &value_v);
}

