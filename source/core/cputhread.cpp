#include <core/cputhread.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <win32/whvemulator.hpp>

using core::CpuThread;

CpuThread::CpuThread()
{}

CpuThread::~CpuThread()
{}

auto CpuThread::Start(Processor& processor_v) -> void {	
	m_Thread = std::jthread(&EntryPoint, std::ref(processor_v));
}

auto CpuThread::Stop() -> void {	
	if (m_Thread.joinable()) {
		m_Thread = std::jthread();
	}
}

auto CpuThread::Suspend() -> void
{}

auto CpuThread::Resume() -> void
{}

auto CpuThread::EntryPoint(std::stop_token stopper_v, Processor& processor_v) -> void
{
	auto const exit_v = processor_v.Run(stopper_v);
}
