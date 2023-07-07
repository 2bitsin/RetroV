#pragma once

#include <thread>
#include <mutex>

namespace core::processor
{
	struct ProcThead
	{
		std::jthread			m_Thread;
	};
}