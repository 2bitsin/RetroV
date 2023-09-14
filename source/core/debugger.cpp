#include <core/debugger.hpp>
#include <core/machine.hpp>

#include <utils/logger.hpp>

#include <charconv>
#include <cassert>

using core::Debugger;

Debugger::Debugger(Machine& machine_v)
	: m_Machine{ machine_v }
{}

auto Debugger::Reset() -> void
{
	std::unique_lock lock{ m_Mutex };
	m_Buffer.clear();
}

auto Debugger::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	return ERROR_ACCESS_DENIED;
}

auto Debugger::Hypercall_UnrealModeEnable(Processor const& vcpu_v, bool enable_v) -> std::int32_t 
{	
	static constexpr const WHV_REGISTER_NAME name_v[] = {
		WHvX64RegisterDs,
		WHvX64RegisterEs,
		WHvX64RegisterFs,
		WHvX64RegisterGs
	};
	WHV_REGISTER_VALUE value_v[4];

	if (enable_v)
	{
		s_log.UnrealModeEnabled(vcpu_v.GetIndex());
		vcpu_v.GetRegisters(name_v, value_v);
		for (auto&& v : value_v) {
			v.Segment.Base = 0u;
			v.Segment.Limit = 0xFFFFFFFFu;
			v.Segment.Attributes = 0xCF93u;
		}
		vcpu_v.SetRegisters(name_v, value_v);
	}
	else 
	{
		s_log.UnrealModeDisabled(vcpu_v.GetIndex());
		vcpu_v.GetRegisters(name_v, value_v);
		for (auto&& v : value_v) {			
		v.Segment.Limit = 0xFFFFu;
			v.Segment.Attributes = 0x0093u;			
		}
		vcpu_v.SetRegisters(name_v, value_v);
	}
	return S_OK;
}

auto Debugger::Hypercall_WriteLogString(Processor const& vcpu_v, std::uint64_t address_v, std::uint64_t length_v) -> std::int32_t
{
	using utils::logger;
	std::vector<std::byte> output_v;
	auto status_v = FetchMemory(vcpu_v, address_v, length_v, output_v);
	if (status_v != ERROR_SUCCESS)
		return status_v;
	std::lock_guard lock{ m_Mutex };
	for (auto char_v : output_v) 
	{		
		if (char_v == std::byte('\n')) {
			logger::debug(logger::deflog, "CPU[{}] : {}", vcpu_v.GetIndex(), m_Buffer);
			m_Buffer.clear();
			continue;
		}
		m_Buffer.push_back((char)char_v);
	}			
	return ERROR_SUCCESS;
}

auto Debugger::Hypercall_DebuggerBreak(Processor const& vcpu_v, std::uint64_t lin_v, std::uint16_t seg_v, std::uint64_t off_v) -> std::int32_t
{
	using utils::logger;
	logger::debug(logger::deflog, "CPU[{}]: DebuggerBreak at ({:08x}) with CS={:04x} IP={:08x}", vcpu_v.GetIndex(), lin_v, seg_v, off_v);	
	__debugbreak();
  return ERROR_SUCCESS;
}

auto Debugger::Hypercall_WriteLogNumber(Processor const& vcpu_v, std::uint32_t valuehi_v, std::uint32_t valuelo_v, std::uint8_t size_v, std::int8_t base_v) -> std::int32_t
{	
	std::uint64_t value_v{ 0 };	
	char buffer_v[128u];
	std::fill(std::begin(buffer_v), 
		std::begin(buffer_v)+64u, 
		'0');
	if (size_v > 32u) {
		value_v |= (valuelo_v * 0x000000001ull);
		value_v |= (valuehi_v * 0x100000000ull);
	} else if (size_v > 16u) {
		value_v |= ((valuelo_v&0xFFFFull) * 0x00001ull);
		value_v |= ((valuehi_v&0xFFFFull) * 0x10000ull);		
	} else {
		value_v = valuelo_v;
	}
	value_v &= ((1ull << size_v) - 1ull);		
	std::to_chars_result result_v{};
	if (base_v < 0) {
		base_v = -base_v;
		result_v = std::to_chars(buffer_v + 64u, 
			buffer_v + sizeof(buffer_v), 
			(std::int64_t)value_v, 
			base_v);
	} else {
		result_v = std::to_chars(buffer_v + 64u, 
			buffer_v + sizeof(buffer_v), 
			(std::uint64_t)value_v, 
			base_v);
	}
	if (result_v.ec == std::errc{}) 
	{
		auto length_v = result_v.ptr - (buffer_v + 64u);		
		if (std::has_single_bit((std::uint8_t)base_v)) {
			auto const digit_width_v = std::bit_width((std::size_t)base_v);
			length_v = (length_v + digit_width_v - 1u) / digit_width_v;
			size_v = (size_v + digit_width_v - 1u) / digit_width_v;
			std::lock_guard lock_v{ m_Mutex };
			m_Buffer.append(buffer_v + 64u - (size_v - length_v), result_v.ptr);
		} else {
			std::lock_guard lock_v{ m_Mutex };
			m_Buffer.append(buffer_v + 64u, result_v.ptr);
		}
	}
	return ERROR_SUCCESS;
}

auto Debugger::Hypercall_WriteLogChar(Processor const& vcpu_v, char value_v) -> std::int32_t
{
	std::lock_guard lock_v{ m_Mutex };
	m_Buffer.push_back(value_v);
	return ERROR_SUCCESS;
}

auto Debugger::FetchMemory(Processor const& vcpu_v, std::uint64_t address_v, std::uint64_t length_v, std::vector<std::byte>& output_v) -> std::int32_t
{
	using std::tie;

	std::uint64_t page_v = address_v & ~0xFFFu;
	std::uint64_t offs_v = address_v & 0xFFFu;
	auto [status_v, xgpa_v] = vcpu_v.TranslateAddress(page_v, Access::kAccessFetch);
	auto const zero_terminated_v = length_v == 0u;
	// Force wrap around to max length
	if (!zero_terminated_v) { 
		output_v.reserve(output_v.size() + length_v);
	} else {
		length_v -= 1u; 
	}

	std::size_t max_bytes_v{ 0 };
	std::byte tmpbuf_v[16u]{ std::byte(0) };	

	while (true)
	{
		max_bytes_v = std::min(std::min(length_v, sizeof(tmpbuf_v)), 0x1000u - offs_v);
		auto buffer_s = utils::as_static_mutable_bytes(tmpbuf_v).first(max_bytes_v);
		status_v = vcpu_v.MemoryAccess(false, xgpa_v + offs_v, buffer_s);

		if (status_v != ERROR_SUCCESS)
			return status_v;

		if (!zero_terminated_v)
			output_v.insert(output_v.end(), tmpbuf_v,
				tmpbuf_v + buffer_s.size());
		else
			for (auto byte_v : tmpbuf_v) {
				if (byte_v == std::byte(0))
					goto Done;
				output_v.push_back(byte_v);
			}

		length_v -= buffer_s.size();
		if (length_v < 1u) break;
		offs_v += buffer_s.size();
		if (offs_v >= 0x1000u) {
			offs_v &= 0xFFFu; page_v += 0x1000u;
			tie(status_v, xgpa_v) = vcpu_v.TranslateAddress(page_v, Access::kAccessFetch);
			if (status_v != ERROR_SUCCESS)
				return status_v;
		}
	}
Done:
	return ERROR_SUCCESS;
}

auto Debugger::Hypercall(Processor const& vcpu_v, HypercallContext const& context_v) -> std::int32_t
{
	auto const& hypercall_v = context_v.Hypercall;
	auto const& vpcontext_v = context_v.VpContext;	

	switch (context_v.Minor) 
	{
	/****************************
	 *	Enable/Disable Unreal Mode
	 ****************************/
	case 0x00:
		if (vpcontext_v.ExecutionState.Cr0Pe||vpcontext_v.ExecutionState.Cpl!=0)
			return ERROR_ACCESS_DENIED;
		return Hypercall_UnrealModeEnable(vcpu_v, !!(hypercall_v.Rbx&1u));

	/****************************
	 *	Write Log String
	 ****************************/
	case 0x01: 
		return Hypercall_WriteLogString(vcpu_v, hypercall_v.Rsi, hypercall_v.Rcx);


 /****************************
	*	Write Log Number
	****************************/
	case 0x02: 
		return Hypercall_WriteLogNumber(vcpu_v, hypercall_v.Rdx, hypercall_v.Rbx, 
			(hypercall_v.Rcx >> 0u)&0xFFu, (hypercall_v.Rcx >> 8u)&0xFFu);


 /****************************
  *	Write Log Char
  ****************************/
	case 0x03:
		return Hypercall_WriteLogChar(vcpu_v, hypercall_v.Rbx&0xFFu);


	/****************************
	 *	Debugger Break
	 ****************************/
	case 0xff: 
		return Hypercall_DebuggerBreak(vcpu_v, vpcontext_v.Cs.Base + vpcontext_v.Rip, 
			vpcontext_v.Cs.Selector, vpcontext_v.Rip);
	}
  return ERROR_ACCESS_DENIED;
}

