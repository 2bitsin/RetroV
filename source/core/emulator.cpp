#include <core/emulator.hpp>
#include <core/processor.hpp>

using core::Emulator;

Emulator::Emulator(Processor& processor_v, SystemBus& systembus_v)
	: m_Processor(&processor_v)
	, m_SystemBus(&systembus_v) 
	, m_EmuHandle(nullptr)
{
	WHV_EMULATOR_CALLBACKS callbacks_v;
	std::memset(&callbacks_v, 0, sizeof(callbacks_v));

	callbacks_v.Size = sizeof(callbacks_v);

	callbacks_v.WHvEmulatorGetVirtualProcessorRegisters = (WHV_EMULATOR_GET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK)&Emulator::GetRegisters;
	callbacks_v.WHvEmulatorSetVirtualProcessorRegisters = (WHV_EMULATOR_SET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK)&Emulator::SetRegisters;
	callbacks_v.WHvEmulatorIoPortCallback = (WHV_EMULATOR_IO_PORT_CALLBACK)&Emulator::IoPortCallback;
	callbacks_v.WHvEmulatorMemoryCallback = (WHV_EMULATOR_MEMORY_CALLBACK)&Emulator::MemoryCallback;
	callbacks_v.WHvEmulatorTranslateGvaPage = (WHV_EMULATOR_TRANSLATE_GVA_PAGE_CALLBACK)&Emulator::TranslateGvaPage;

	WIN32_ERROR_ASSERT(::WHvEmulatorCreateEmulator(&callbacks_v, &m_EmuHandle));
}

auto Emulator::GetRegisters(Emulator& context_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT
{
	return S_OK;
}

auto Emulator::SetRegisters(Emulator& context_v, WHV_REGISTER_NAME const* names_v, uint32_t count_v, WHV_REGISTER_VALUE const* values_v) -> HRESULT
{
	return E_NOTIMPL;
}

Emulator::~Emulator()
{
	if(nullptr!=m_EmuHandle) {
		::WHvEmulatorDestroyEmulator(m_EmuHandle);
	}
}
