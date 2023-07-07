#include <core/hypervisor.hpp>
#include <core/processor/flags.hpp>
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
	:	m_Partition		{ *this }
	,	m_Scheduler		{ *this }
	,	m_MemPool			{ *this }
	,	m_MemManager	{ *this }
	,	m_Processors	{ }
	,	m_Debugger		{ *this }
	, m_EventBroker { *this }
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

auto Hypervisor::GetEventBroker() -> EventBroker&
{
	return m_EventBroker;
}

auto Hypervisor::GetMemPool() -> mem::Pool&
{
	return m_MemPool;
}

auto Hypervisor::GetMemManager() -> mem::Manager&
{
	return m_MemManager;
}

auto Hypervisor::GetProcessor(std::uint32_t index_v)->Processor& 
{
	auto position_v = std::lower_bound(m_Processors.begin(), m_Processors.end(), index_v, 
		[](auto const& processor_v, auto const& index_v) {
			return processor_v.GetIndex() < index_v; });
	if (position_v == m_Processors.end() || position_v->GetIndex() != index_v) {
		throw std::out_of_range{ "Invalid processor index" }; }
	return *position_v;
}

auto Hypervisor::PowerOn() -> void
{
	GetScheduler().DeviceResumeAll();
	for (auto& processor_v : m_Processors) {
		processor_v.RunInThread(GetEventBroker()); }
}

auto Hypervisor::Shutdown() -> void
{
	for (auto& processor_v : m_Processors) {
		processor_v.CancelRun(); }
	GetScheduler().DevicePauseAll();	
}

auto Hypervisor::InitializePartition() -> void
{
	auto& partition_v = GetPartition();
	partition_v.SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, 
		std::uint64_t{ 1u << WHvX64ExceptionTypeInvalidOpcodeFault });
	partition_v.SetProperty(WHvPartitionPropertyCodeExtendedVmExits, 
		WHV_EXTENDED_VM_EXITS{ .ExceptionExit = 1, .HypercallExit = 1 });
	partition_v.SetProperty(WHvPartitionPropertyCodeProcessorFeatures, 
		WHV_PROCESSOR_FEATURES{ .LahfSahfSupport = 1 });
}

auto Hypervisor::InitializeProcessor(std::uint32_t index_v) -> void
{
	auto position_v = std::lower_bound(m_Processors.begin(), m_Processors.end(), index_v, 
		[](auto&& processor_v, auto&& index_v) {
			return processor_v.GetIndex() < index_v;
		});
	m_Processors.emplace(position_v, *this, index_v);
}
