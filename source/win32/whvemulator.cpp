#include <win32/whvemulator.hpp>

#include <cassert>

#include <utility>
using std::exchange;

using win32::WHvEmulator;

WHvEmulator::WHvEmulator(WHV_EMULATOR_HANDLE handle_v) noexcept
	: m_Handle{ handle_v }
{}

WHvEmulator::WHvEmulator()
	: WHvEmulator(nullptr)
{}

WHvEmulator::~WHvEmulator() {
	if (nullptr != m_Handle) {
		::WHvDeleteEmulator(m_Handle);
	}
}

WHvEmulator::WHvEmulator(WHvEmulator&& other_v) noexcept
	: m_Handle{ exchange(other_v.m_Handle, nullptr) }
{}

auto WHvEmulator::operator=(WHvEmulator&& other_v) noexcept -> WHvEmulator&
{
	if (this != &other_v) {
		auto temp_v{ std::move(other_v) };
		temp_v.swap(*this);
	}
	return *this;	
}

auto WHvEmulator::swap(WHvEmulator& other_v) noexcept -> void
{
	std::swap(m_Handle, other_v.m_Handle);
}

auto WHvEmulator::GetHandle() const noexcept -> WHV_EMULATOR_HANDLE
{
	return m_Handle;
}

auto WHvEmulator::Create() -> WHvEmulator
{
	WHV_EMULATOR_HANDLE handle_v{ nullptr };
	WHV_EMULATOR_CALLBACKS callbacks_v{ 0 };
	callbacks_v.Size = sizeof(WHV_EMULATOR_CALLBACKS);
	callbacks_v.WHvEmulatorGetVirtualProcessorRegisters = &GetRegisters;
	callbacks_v.WHvEmulatorSetVirtualProcessorRegisters = &SetRegisters;
	callbacks_v.WHvEmulatorTranslateGvaPage = &TranslateGvaPage;
	callbacks_v.WHvEmulatorIoPortCallback = &IoPortAccess;
	callbacks_v.WHvEmulatorMemoryCallback = &MemoryAccess;
	WIN32_ERROR_ASSERT(::WHvEmulatorCreateEmulator(&callbacks_v, &handle_v));
	return WHvEmulator(handle_v);
}

auto WHvEmulator::MemoryAccess(void* context_v, WHV_EMULATOR_MEMORY_ACCESS_INFO* info_v, -> HRESULT
{	
	assert(((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorMemoryCallback != nullptr);
	return ((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorMemoryCallback(context_v, info_v);
}

auto WHvEmulator::IoPortAccess(void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* info_v) -> HRESULT
{
	assert(((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorIoPortCallback != nullptr);
	return ((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorIoPortCallback(context_v, info_v);
}

auto WHvEmulator::GetRegisters(void* context_v, const WHV_REGISTER_NAME* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v)
{
	assert(((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorGetVirtualProcessorRegisters != nullptr);
	return ((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorGetVirtualProcessorRegisters(context_v, names_v, count_v, values_v);
}

auto WHvEmulator::SetRegisters(void* context_v, const WHV_REGISTER_NAME* names_v, uint32_t count_v, const WHV_REGISTER_VALUE* values_v)
{
	assert(((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorSetVirtualProcessorRegisters != nullptr);
	return ((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorSetVirtualProcessorRegisters(context_v, names_v, count_v, values_v);
}

auto WHvEmulator::TranslateGvaPage(void* context_v, uint64_t gva_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE* result_v, WHV_GPA* gpa_v) -> HRESULT
{
	assert(((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorTranslateGvaPage != nullptr);
	return ((WHV_EMULATOR_CALLBACKS const*)context_v)->WHvEmulatorTranslateGvaPage(context_v, gva_v, flags_v, result_v, gpa_v);
}
