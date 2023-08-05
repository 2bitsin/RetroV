#include <core/cputhread.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <win32/whvemulator.hpp>

using core::CpuThread;

CpuThread::CpuThread()
{}

CpuThread::~CpuThread()
{}

auto CpuThread::Start(Machine& machine_v) -> void {	
	m_Thread = std::jthread(&EntryPoint, std::ref(machine_v), std::ref(*this));
}

auto CpuThread::Stop() -> void {	
	if (m_Thread.joinable()) {
		m_Thread = std::jthread();
	}
}

auto CpuThread::Suspend() -> void
{
	m_Barrier.lock(true);
}

auto CpuThread::Resume() -> void
{
	m_Barrier.lock(false);
}

auto CpuThread::EntryPoint(std::stop_token stopper_v, Machine& machine_v, CpuThread& this_v) -> void
{
	auto& processor_v = machine_v.Processor();
	auto& emulator_v = machine_v.Emulator();

	std::stop_callback stopreq_v(stopper_v,
		[&processor_v]{processor_v.Cancel();});

	while (!stopper_v.stop_requested())
	{
		this_v.m_Barrier.wait();
		auto const [result_v, exit_v] = processor_v.Run();
		WIN32_ERROR_ASSERT(result_v);
		WHV_REGISTER_VALUE scratch_v { };
		switch (exit_v.ExitReason)
		{
		case WHvRunVpExitReasonX64IoPortAccess:
			emulator_v.TryIoEmulation(processor_v, exit_v.VpContext, exit_v.IoPortAccess);
			continue;
		case WHvRunVpExitReasonX64MsrAccess:
			emulator_v.TryMmioEmulation(processor_v, exit_v.VpContext, exit_v.MemoryAccess);
			continue;
		case WHvRunVpExitReasonCanceled:
			continue;
		case WHvRunVpExitReasonX64Halt:
			{
				WHV_REGISTER_VALUE flags_v { };
				processor_v.GetRegister(WHvX64RegisterRflags, flags_v);
				this_v.Suspend();
			}
			continue;
		default:
			__debugbreak();
			throw std::runtime_error("Unhandled exit reason");
		}
	}
}
