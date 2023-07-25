#include <core/processorthread.hpp>
#include <win32/whvprocessor.hpp>
#include <utils/lambda.hpp>

#include <iostream>

using core::ProcessorThread;

ProcessorThread::~ProcessorThread() noexcept
{
	Stop();
}

auto ProcessorThread::Start(VirtualMachine& machine_v, win32::WHvProcessor processor_v) -> void
{
	Stop();	
	using utils::lambda;
	m_Thread = std::jthread(lambda(this, &ProcessorThread::RunProcessor),
		std::ref(machine_v), std::move(processor_v));
}

auto ProcessorThread::Stop() -> void
{
	if (m_Thread.joinable())
	{
		m_Thread.request_stop();
		m_Thread.join();
	}
}

auto ProcessorThread::RunProcessor(std::stop_token token_v, VirtualMachine& machine_v, win32::WHvProcessor processor_v) -> void
{
	try
	{
		std::stop_callback callback_v(token_v, [&processor_v] { 
			processor_v.Cancel(); 
		});

		while (!token_v.stop_requested()) {
			WHV_RUN_VP_EXIT_CONTEXT exit_v { };
			WIN32_ERROR_ASSERT(processor_v.Run(exit_v));	
			machine_v.ProcessorExit(exit_v, processor_v);
		}
	}
	catch (std::exception const& e)
	{
		std::cout << "ProcessorThread::RunProcessor: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "ProcessorThread::RunProcessor: Unknown exception" << std::endl;
	}
}

