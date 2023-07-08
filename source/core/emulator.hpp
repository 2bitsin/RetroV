#pragma once

#include <win32/windows.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>

#include <core/processor_fwd.hpp>
#include <core/systembus_fwd.hpp>

namespace core
{
	struct Emulator
	{
		Emulator(Processor&, SystemBus&);
		static auto __stdcall GetRegisters(Emulator& context_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v)->HRESULT;
		static auto __stdcall SetRegisters(Emulator& context_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE const* values_v)->HRESULT;
		~Emulator();

	protected:

	private:		
		Processor *m_Processor;
		SystemBus *m_SystemBus;
		WHV_EMULATOR_HANDLE m_EmuHandle;
	};

}