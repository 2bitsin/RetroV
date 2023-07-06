#include <core/eventbroker.hpp>

using core::EventBroker;

EventBroker::EventBroker(Hypervisor& hypervisor_v)
	: m_Hypervisor(&hypervisor_v)
	, m_DiaptchTbl(std::make_unique<DispatchTables>())
{}

EventBroker::EventBroker(EventBroker&& prev_v) noexcept 
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_DiaptchTbl(std::exchange(prev_v.m_DiaptchTbl, std::make_unique<DispatchTables>()))
{}

auto EventBroker::operator=(EventBroker&& prev_v) noexcept -> EventBroker& {
	if (this != &prev_v) {		
		auto tmp_v{ std::move(prev_v) };		
		tmp_v.Swap(*this);
	}
	return *this;
}

auto EventBroker::Swap(EventBroker& other_v) noexcept -> void
{
	using std::swap;
	swap(m_Hypervisor, other_v.m_Hypervisor);
	swap(m_DiaptchTbl, other_v.m_DiaptchTbl);
}

EventBroker::~EventBroker()
{}

auto EventBroker::DispatchEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	auto& self{ *m_DiaptchTbl };
	switch (exit_v.ExitReason) 
	{
	case WHvRunVpExitReasonX64IoPortAccess:
		break;
	case WHvRunVpExitReasonHypercall:
		break;
	case WHvRunVpExitReasonMemoryAccess:
		break;
	default:
		break;
	}
	return false;
}

auto EventBroker::SetupRange(std::uint64_t base_v, std::size_t size_v, event_handler handler_v, IOType iotype_v) -> void
{
}

auto EventBroker::ClearRange(std::uint64_t base_v, std::size_t size_v, IOType iotype_v) -> void
{
}

auto EventBroker::DispatchIoAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	return false;
}

auto EventBroker::DispatchMemAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	return false;
}

auto EventBroker::DispatchVMMCallEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	return false;
}
