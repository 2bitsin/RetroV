#pragma once

#include <win32/windows.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>

#include <core/hypervisor_fwd.hpp>
#include <core/processor_fwd.hpp>
#include <core/systembus_fwd.hpp>

namespace core
{
	struct Emulator
	{
		Emulator();
		~Emulator();

	protected:
		static auto __stdcall GetRegisters(Processor& vcpu_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT;
		static auto __stdcall SetRegisters(Processor& vcpu_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE const* values_v) -> HRESULT;
		static auto __stdcall TranslateGva(Processor& vcpu_v, uint64_t vaddress_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT& code_v, uint64_t& paddress_v) -> HRESULT;
		static auto __stdcall IoPortAccess(Processor& vcpu_v, WHV_EMULATOR_IO_ACCESS_INFO& context_v) -> HRESULT;
		static auto __stdcall MemoryAccess(Processor& vcpu_v, WHV_EMULATOR_MEMORY_ACCESS_INFO& context_v) -> HRESULT;

	private:		
		WHV_EMULATOR_HANDLE m_EmuHandle;
	};

}