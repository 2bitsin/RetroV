#include <core/eventbroker.hpp>
#include <core/cpu/processor.hpp>
#include <core/hypervisor.hpp>

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

auto EventBroker::SetupRange(std::uint64_t base_v, std::size_t size_v, event_handler handler_v, IOType iotype_v) -> void {
	using w = std::uint16_t;
	if (iotype_v & IOType::kIoFetch) {
		auto& table_v{ (*m_DiaptchTbl).m_IOFetch };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v); }

	if (iotype_v & IOType::kIoWrite) {
		auto& table_v{ (*m_DiaptchTbl).m_IOFetch };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v); }

	if (iotype_v & IOType::kVMCall) {
		auto& table_v{ (*m_DiaptchTbl).m_VMMCall };
		table_v.insert({ w(base_v), w(base_v + size_v) }, handler_v); }

	if (iotype_v & IOType::kMemFetch) {
		auto& table_v{ (*m_DiaptchTbl).m_MemFetch };
		table_v.insert({ base_v, base_v + size_v }, handler_v); }

	if (iotype_v & IOType::kMemWrite) {
		auto& table_v{ (*m_DiaptchTbl).m_MemWrite };
		table_v.insert({ base_v, base_v + size_v }, handler_v); }
}

auto EventBroker::ClearRange(std::uint64_t base_v, std::size_t size_v, IOType iotype_v) -> void {
	static const event_handler nullop_handler = [](auto&&...) { return false; };
	using w = std::uint16_t;
	if (iotype_v & IOType::kIoFetch) {
		auto& table_v{ (*m_DiaptchTbl).m_IOFetch };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }

	if (iotype_v & IOType::kIoWrite) {
		auto& table_v{ (*m_DiaptchTbl).m_IOFetch };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }

	if (iotype_v & IOType::kVMCall) {
		auto& table_v{ (*m_DiaptchTbl).m_VMMCall };
		table_v.erase(std::get<1>(table_v.insert({ w(base_v), w(base_v + size_v) }, nullop_handler))); }

	if (iotype_v & IOType::kMemFetch) {
		auto& table_v{ (*m_DiaptchTbl).m_MemFetch };
		table_v.erase(std::get<1>(table_v.insert({ base_v, base_v + size_v }, nullop_handler))); }

	if (iotype_v & IOType::kMemWrite) {
		auto& table_v{ (*m_DiaptchTbl).m_MemWrite };
		table_v.erase(std::get<1>(table_v.insert({ base_v, base_v + size_v }, nullop_handler))); }
}

auto EventBroker::DispatchIoAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
	static const event_handler nullop_handler = [](auto&&...) { return false; };

	auto const& iowtbl_v{ (*m_DiaptchTbl).m_IOWrite };
	auto const& iortbl_v{ (*m_DiaptchTbl).m_IOFetch };
	auto const& access_v{ exit_v.IoPortAccess };
	auto const& ioinfo_v{ access_v.AccessInfo };	
	auto const port_v = access_v.PortNumber;

	if (ioinfo_v.IsWrite) {
		auto const& handler_v = iowtbl_v.at(port_v, nullop_handler);
		return handler_v(processor_v, exit_v);
	} else {
		auto const& handler_v = iortbl_v.at(port_v, nullop_handler);
		return handler_v(processor_v, exit_v);
	}

	return false;
}

auto EventBroker::DispatchVMMCallEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
	static const event_handler nullop_handler = [](auto&&...) { return false; };

	auto const& vmctbl_v{ (*m_DiaptchTbl).m_VMMCall };
	auto const& access_v{ exit_v.Hypercall };
	auto const& vpinfo_v{ exit_v.VpContext };

	auto& memm_v = (*m_Hypervisor).GetMemManager();	

	auto opcode_v = memm_v.FetchValue<uint32_t>(
		processor_v.GetIndex(), 
		vpinfo_v.Cs.Base + vpinfo_v.Rip - 4u);

	std::uint16_t callno_v{ 0xFFFFu };
	if (0xEB020000u==(opcode_v&0xFFFF0000u)) {
		callno_v=opcode_v&0xFFFFu;
	}

	auto const& handler_v = vmctbl_v.at(callno_v, nullop_handler);

	return handler_v(processor_v, exit_v);
}

auto EventBroker::DispatchMemAccessEvent(Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool {
static const event_handler nullop_handler = [](auto&&...) { return false; };

	auto const& mwatbl_v{ (*m_DiaptchTbl).m_MemWrite };
	auto const& mratbl_v{ (*m_DiaptchTbl).m_MemFetch };

	auto const& access_v{ exit_v.MemoryAccess };
	auto const& vpinfo_v{ exit_v.VpContext };
	auto const& mainfo_v{ access_v.AccessInfo };

	auto& memm_v = (*m_Hypervisor).GetMemManager();

	if (mainfo_v.AccessType == WHvMemoryAccessRead || mainfo_v.AccessType == WHvMemoryAccessExecute) {
		auto const& handler_v = mratbl_v.at(access_v.Gpa, nullop_handler);
		return handler_v(processor_v, exit_v); } 
	else if (mainfo_v.AccessType == WHvMemoryAccessWrite) {
		auto const& handler_v = mwatbl_v.at(access_v.Gpa, nullop_handler);
		return handler_v(processor_v, exit_v); }

	return false;
}
