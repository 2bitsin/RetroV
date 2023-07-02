#include <core/hypervisor.hpp>
#include <core/cpu/flags.hpp>
#include <win32/error.hpp>
#include <utils/bitmanip.hpp>
#include <utils/capstone.hpp>

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <format>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>


using core::Hypervisor;

Hypervisor::Hypervisor(Config const& config_v)
	:	m_Partition  { *this }
	,	m_Scheduler  { *this }
	,	m_MemPool    { *this }
	,	m_MemManager { *this }
	, m_IoManager  { *this }
	,	m_VcManager  { *this }
	,	m_Processors { }
	,	m_Debugger   { *this }
{
	InitializePartition();
	config_v.ApplyBeforeSetup(*this);
	m_Partition.Setup();
	config_v.ApplyAfterSetup(*this);
}

Hypervisor::~Hypervisor()
{}

auto Hypervisor::GetParitionHandle() -> WHV_PARTITION_HANDLE
{
	return GetPartition().GetHandle();
}

auto Hypervisor::GetScheduler() -> Scheduler&
{
  return m_Scheduler;
}

auto Hypervisor::GetPartition() -> core::Partition&
{
  return m_Partition;
}

auto Hypervisor::GetMemPool() -> mem::Pool&
{
	return m_MemPool;
}

auto Hypervisor::GetMemManager() -> mem::Manager&
{
	return m_MemManager;
}

auto Hypervisor::GetIoManager() -> io::Manager&
{
	return m_IoManager;
}

auto Hypervisor::GetVcManager() -> vmc::Manager&
{
	return m_VcManager;
}

auto Hypervisor::GetProcessor(std::uint32_t index_v)->cpu::Processor& 
{
	auto position_v = std::lower_bound(m_Processors.begin(), m_Processors.end(), index_v, 
		[](auto const& processor_v, auto const& index_v) {
			return processor_v.GetIndex() < index_v; });
	if (position_v == m_Processors.end() || position_v->GetIndex() != index_v) {
		throw std::out_of_range{ "Invalid processor index" }; }
	return *position_v;
}

auto Hypervisor::DispatchHalt(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool 
{
	if (exit_v.VpContext.Rflags & cpu::kInterruptFlag) {
		// Interrupts enabled
		__debugbreak();
		return true;
	}

	return false;
}


auto Hypervisor::DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{	
	switch (exit_v.ExitReason) 
	{
	case WHvRunVpExitReasonX64Halt:
		return DispatchHalt(processor_v, exit_v);
	case WHvRunVpExitReasonX64IoPortAccess:
		return m_IoManager.DispatchExit(processor_v, exit_v);
	case WHvRunVpExitReasonHypercall:	
		return m_VcManager.DispatchExit(processor_v, exit_v);
	default:
		__debugbreak();
		m_Debugger.PrintRegisters(std::cerr, processor_v.GetRegisters());
		m_Debugger.Disassemble(std::cerr, processor_v, exit_v.VpContext.Rip+exit_v.VpContext.Cs.Base, 20u);
		__debugbreak();
		return false;
	}
	return true;
}

auto Hypervisor::NextInstruction(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> void {	
	processor_v.SetRegister(WHvX64RegisterRip, exit_v.VpContext.Rip + exit_v.VpContext.InstructionLength);
}


auto Hypervisor::Run() -> void
{
	auto& processor_v = m_Processors[0];
	auto future_v = processor_v.RunAsync();
		
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(10ms);

	auto exit_v = future_v.get();

	__debugbreak();
}

auto Hypervisor::InitializePartition() -> void
{
	auto scheduler_features_v = GetCapability<WHV_CAPABILITY_PROCESSOR_FREQUENCY_CAP>(WHvCapabilityCodeProcessorFrequencyCap);

	auto& partition_v = GetPartition();
	partition_v.SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, std::uint64_t{ 0x40u });
	partition_v.SetProperty(WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .ExceptionExit = 1, .HypercallExit = 1 });
	partition_v.SetProperty(WHvPartitionPropertyCodeProcessorFeatures, WHV_PROCESSOR_FEATURES{ .LahfSahfSupport = 1 });
}

auto Hypervisor::GetCapability(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto Hypervisor::InitializeProcessor(std::uint32_t index_v) -> void
{
	auto position_v = std::lower_bound(m_Processors.begin(), m_Processors.end(), index_v, 
		[](auto&& processor_v, auto&& index_v) {
			return processor_v.GetIndex() < index_v;
		});
	m_Processors.emplace(position_v, *this, index_v);
}
