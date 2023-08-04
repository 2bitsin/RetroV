#include <core/debugger.hpp>

#include <utils/logger.hpp>

#include <cassert>

using core::Debugger;

auto Debugger::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	using utils::logger;
	switch (port_v)
	{
	case 0xe9: 
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
		return S_OK;	
	}

	return S_OK;
}

auto Debugger::Reset() -> void
{
	std::unique_lock lock(m_Mutex);
	m_Buffer.clear();
}
