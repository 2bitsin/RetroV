#include <core/eventbroker.hpp>
#include <core/processor.hpp>
#include <core/hypervisor.hpp>

using core::EventBroker;

EventBroker::EventBroker(Hypervisor& hypervisor_v)
	: m_Hypervisor(&hypervisor_v)
	, m_DispatchTbl(std::make_unique<DispatchTables>())
{}

EventBroker::EventBroker(EventBroker&& prev_v) noexcept
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_DispatchTbl(std::exchange(prev_v.m_DispatchTbl, std::make_unique<DispatchTables>()))
{}

auto EventBroker::operator=(EventBroker&& prev_v) noexcept -> EventBroker& {
	if (this==&prev_v) 
		return *this;	
	auto tmp_v{ std::move(prev_v) };
	tmp_v.Swap(*this);
	return *this;
}

auto EventBroker::Swap(EventBroker& other_v) noexcept -> void
{
	using std::swap;
	swap(m_Hypervisor, other_v.m_Hypervisor);
	swap(m_DispatchTbl, other_v.m_DispatchTbl);
}

EventBroker::~EventBroker()
{}

auto EventBroker::DispatchEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	auto& self{ *m_DispatchTbl };
	switch (exit_v.ExitReason) 
	{
	case WHvRunVpExitReasonX64IoPortAccess: 
		return DispatchIoAccessEvent(processor_v, exit_v);
	case WHvRunVpExitReasonHypercall:
		return DispatchHypercallEvent(processor_v, exit_v);
	case WHvRunVpExitReasonMemoryAccess:
		return DispatchMemAccessEvent(processor_v, exit_v);
	case WHvRunVpExitReasonCanceled:
		return false;
	default:
		__debugbreak();
		break;
	}
	return false;
}

auto EventBroker::SetupRange(std::uint64_t base_v, std::size_t size_v, event_handler handler_v, IOType iotype_v) -> void {
	using w = std::uint16_t;
	if (iotype_v & IOType::kIoFetch) {
		auto& table_v{ (*m_DispatchTbl).m_IOFetch };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v);
	}
	if (iotype_v & IOType::kIoWrite) {
		auto& table_v{ (*m_DispatchTbl).m_IOWrite };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v);
	}
	if (iotype_v & IOType::kVMCall) {
		auto& table_v{ (*m_DispatchTbl).m_Hypercall };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v);
	}
	if (iotype_v & IOType::kMemFetch) {
		auto& table_v{ (*m_DispatchTbl).m_MemFetch };
		table_v.insert({ base_v, base_v + size_v }, handler_v);
	}
	if (iotype_v & IOType::kMemWrite) {
		auto& table_v{ (*m_DispatchTbl).m_MemWrite };
		table_v.insert({ base_v, base_v + size_v }, handler_v);
	}
}

auto EventBroker::ClearRange(std::uint64_t base_v, std::size_t size_v, IOType iotype_v) -> void {
	static const event_handler nullop_handler = [](auto&&...) { return false; };
	using w = std::uint16_t;

	std::unique_lock lock_v{ m_DispatchTbl->m_Mutex };
	if (iotype_v & IOType::kIoFetch) {
		auto& table_v{ (*m_DispatchTbl).m_IOFetch };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }
	if (iotype_v & IOType::kIoWrite) {
		auto& table_v{ (*m_DispatchTbl).m_IOWrite };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }
	if (iotype_v & IOType::kVMCall) {
		auto& table_v{ (*m_DispatchTbl).m_Hypercall };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }
	if (iotype_v & IOType::kMemFetch) {
		auto& table_v{ (*m_DispatchTbl).m_MemFetch };
		table_v.erase(std::get<1>(table_v.insert({ base_v, base_v + size_v }, nullop_handler))); }
	if (iotype_v & IOType::kMemWrite) {
		auto& table_v{ (*m_DispatchTbl).m_MemWrite };
		table_v.erase(std::get<1>(table_v.insert({ base_v, base_v + size_v }, nullop_handler))); }
}

auto EventBroker::DispatchIoAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
	static const event_handler nullop_handler = [](auto&&...) { return false; };	
	auto const& iowtbl_v{ (*m_DispatchTbl).m_IOWrite };
	auto const& iortbl_v{ (*m_DispatchTbl).m_IOFetch };
	auto const& access_v{ exit_v.IoPortAccess };
	auto const& ioinfo_v{ access_v.AccessInfo };
	auto const port_v = access_v.PortNumber;
	std::shared_lock lock_v{ m_DispatchTbl->m_Mutex };
	auto const& handler_v = ioinfo_v.IsWrite 
		? iowtbl_v.at(port_v, nullop_handler) 
		: iortbl_v.at(port_v, nullop_handler) ;
	lock_v.unlock();
	return handler_v(processor_v, exit_v);
}

auto EventBroker::DispatchHypercallEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
	static const event_handler nullop_handler = [](auto&&...) { return false; };

	auto const& vmctbl_v{ (*m_DispatchTbl).m_Hypercall };
	auto const& access_v{ exit_v.Hypercall };
	auto const& vpinfo_v{ exit_v.VpContext };
	auto& memm_v = (*m_Hypervisor).GetMemManager();

	auto opcode_v = memm_v.FetchValue<uint32_t>(
		processor_v.GetIndex(),
		vpinfo_v.Cs.Base + vpinfo_v.Rip - 4u);

	std::uint16_t callno_v{ 0xFFFFu };
	if (0xEB020000u == (opcode_v & 0xFFFF0000u)) {
		callno_v = opcode_v & 0xFFFFu;
	}

	std::shared_lock lock_v{ m_DispatchTbl->m_Mutex };
	auto const& handler_v = vmctbl_v.at(callno_v, nullop_handler);
	lock_v.unlock();
	return handler_v(processor_v, exit_v);
}

auto EventBroker::DispatchMemAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
	static const event_handler nullop_handler = [](auto&&...) { return false; };

	auto const& mwatbl_v{ (*m_DispatchTbl).m_MemWrite };
	auto const& mratbl_v{ (*m_DispatchTbl).m_MemFetch };
	auto const& access_v{ exit_v.MemoryAccess };
	auto const& vpinfo_v{ exit_v.VpContext };
	auto const& mainfo_v{ access_v.AccessInfo };
	auto& memm_v = (*m_Hypervisor).GetMemManager();

	std::shared_lock lock_v{ m_DispatchTbl->m_Mutex };
	auto const& handler_v = mainfo_v.AccessType == WHvMemoryAccessWrite 
		? mwatbl_v.at(access_v.Gpa, nullop_handler)
		: mratbl_v.at(access_v.Gpa, nullop_handler);
	lock_v.unlock();
	return handler_v(processor_v, exit_v);
}
