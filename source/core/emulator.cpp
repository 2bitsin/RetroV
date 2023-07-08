#include <core/emulator.hpp>
#include <core/processor.hpp>

using core::Emulator;

Emulator::Emulator(): m_EmuHandle(nullptr)
{
	WHV_EMULATOR_CALLBACKS callbacks_v;
	std::memset(&callbacks_v, 0, sizeof(callbacks_v));

	callbacks_v.Size = sizeof(callbacks_v);

	callbacks_v.WHvEmulatorGetVirtualProcessorRegisters = 
		(WHV_EMULATOR_GET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK)&Emulator::GetRegisters;

	callbacks_v.WHvEmulatorSetVirtualProcessorRegisters = 
		(WHV_EMULATOR_SET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK)&Emulator::SetRegisters;

	callbacks_v.WHvEmulatorTranslateGvaPage = 
		(WHV_EMULATOR_TRANSLATE_GVA_PAGE_CALLBACK)&Emulator::TranslateGva;

	callbacks_v.WHvEmulatorIoPortCallback = 
		(WHV_EMULATOR_IO_PORT_CALLBACK)&Emulator::IoPortAccess;

	callbacks_v.WHvEmulatorMemoryCallback = 
		(WHV_EMULATOR_MEMORY_CALLBACK)&Emulator::MemoryAccess;	
	
	WIN32_ERROR_ASSERT(::WHvEmulatorCreateEmulator(&callbacks_v, &m_EmuHandle));
}

auto Emulator::GetRegisters(Processor& vcpu_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT {
	return ::WHvGetVirtualProcessorRegisters(vcpu_v.GetPartitionHandle(), vcpu_v.GetIndex(), names_v, count_v, values_v);
}

auto Emulator::SetRegisters(Processor& vcpu_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE const* values_v) -> HRESULT {
	return ::WHvSetVirtualProcessorRegisters(vcpu_v.GetPartitionHandle(), vcpu_v.GetIndex(), names_v, count_v, values_v);
}

auto Emulator::TranslateGva(Processor& vcpu_v, uint64_t vaddress_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT& code_v, uint64_t& paddress_v) -> HRESULT {
	return ::WHvTranslateGva(vcpu_v.GetPartitionHandle(), vcpu_v.GetIndex(), vaddress_v, flags_v, &code_v, &paddress_v);
}

auto Emulator::IoPortAccess(Processor& vcpu_v, WHV_EMULATOR_IO_ACCESS_INFO& context_v) -> HRESULT {
	return vcpu_v.IoPortAccess(context_v) ? S_OK : E_FAIL;
}

auto Emulator::MemoryAccess(Processor& vcpu_v, WHV_EMULATOR_MEMORY_ACCESS_INFO& context_v) -> HRESULT {
	return vcpu_v.MemoryAccess(context_v) ? S_OK : E_FAIL;
}



Emulator::~Emulator()
{
	if(nullptr!=m_EmuHandle) {
		::WHvEmulatorDestroyEmulator(m_EmuHandle);
	}
}
