#include <core/cputhread.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <win32/whvemulator.hpp>

using core::CpuThread;

CpuThread::CpuThread()
{}

CpuThread::~CpuThread()
{}

auto CpuThread::Start(Machine& machine_v) -> void
{	
	m_Thread = std::jthread(&EntryPoint, std::ref(machine_v));
}

auto CpuThread::Stop() -> void
{	
	m_Thread = std::jthread();
}

auto CpuThread::EntryPoint(std::stop_token stopper_v, Machine& machine_v) -> void
{
	auto& processor_v = machine_v.Processor();
	auto& emulator_v = machine_v.Emulator();

	std::stop_callback stopreq_v(stopper_v,
		[&processor_v]{processor_v.Cancel();});

	while (!stopper_v.stop_requested())
	{
		auto const [result_v, exit_v] = processor_v.Run();
		WIN32_ERROR_ASSERT(result_v);
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
		default:
			__debugbreak();
			throw std::runtime_error("Unhandled exit reason");
		}
	}
}
