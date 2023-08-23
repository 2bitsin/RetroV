#include <core/debugger.hpp>
#include <core/machine.hpp>

#include <utils/logger.hpp>

#include <cassert>

using core::Debugger;

Debugger::Debugger(Machine& machine_v)
	: m_Machine{ machine_v }
{}

auto Debugger::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	using utils::logger;
	switch (port_v)
	{
	case 0x00u:
		if (is_write_v) 
		{			
			switch(data_v.size())
			{
			case 1: logger::debug(logger::deflog, "DebugPort[0] <= {:#04x}",  data_v.as<uint8_t >()) ; break;
			case 2: logger::debug(logger::deflog, "DebugPort[0] <= {:#06x}",  data_v.as<uint16_t>()) ; break;
			case 4: logger::debug(logger::deflog, "DebugPort[0] <= {:#010x}", data_v.as<uint32_t>()) ; break;
			}			
		}
		return S_OK;
	case 0x01u: 
		{
			if (!is_write_v) {
				std::fill (data_v.begin(), data_v.end(), std::byte{});
				std::unique_lock lock(m_Mutex);
				logger::debug(logger::deflog, "{}", m_Buffer);
				m_Buffer.clear();
				return S_OK;
			}
			assert(data_v.size() >= 1u);		
			std::unique_lock lock(m_Mutex);
			if (auto const char_v = static_cast<char>(data_v[0]); 
				char_v != '\n' && char_v != '\r') 
			{
				m_Buffer.push_back(char_v);
			} else {
				if (m_Buffer.empty())
					return S_OK;
				logger::debug(logger::deflog, "{}", m_Buffer);
				m_Buffer.clear();
			}
		}
		return S_OK;	
	case 0x02u: 
		WHV_REGISTER_VALUE reg_v{ };
		if (is_write_v) {
			logger::info(logger::deflog, "CPU[{}] flat real mode hack enabled!", vcpu_v.GetIndex());
			WHV_REGISTER_NAME name_v[] = {
				WHvX64RegisterDs,
				WHvX64RegisterEs,
				WHvX64RegisterFs,
				WHvX64RegisterGs
			};
			WHV_REGISTER_VALUE value_v[4];
			vcpu_v.GetRegisters(name_v, value_v);
			for (auto&& v : value_v) {
				v.Segment.Base = 0u;
				v.Segment.Limit = 0xFFFFFFFFu;
				v.Segment.Attributes = 0xCF93u;
			}
			vcpu_v.SetRegisters(name_v, value_v);
		}
	break;
	}

	return S_OK;
}

auto Debugger::Reset() -> void
{
	std::unique_lock lock(m_Mutex);
	m_Buffer.clear();
}
