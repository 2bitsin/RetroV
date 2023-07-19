#include <win32/whvemulator.hpp>

#include <cassert>

#include <utility>
using std::exchange;

using win32::WHvEmulator;

WHvEmulator::WHvEmulator(WHV_EMULATOR_HANDLE handle_v) noexcept
	: m_Handle{ handle_v }
{}

auto win32::WHvEmulator::TryIoEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_X64_IO_PORT_ACCESS_CONTEXT const& ioctx_v) noexcept 
	-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>
{
	WHV_EMULATOR_STATUS status_v;
  auto result_v = WHvEmulatorTryIoEmulation(m_Handle, context_v, &vpctx_v, &ioctx_v, &status_v);
	return{ result_v, status_v };
}

auto win32::WHvEmulator::TryMmioEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_MEMORY_ACCESS_CONTEXT const& mmctx_v) noexcept 
	-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>
{
  WHV_EMULATOR_STATUS status_v;
	auto result_v = WHvEmulatorTryMmioEmulation(m_Handle, context_v, &vpctx_v, &mmctx_v, &status_v);
	return{ result_v, status_v };
}

WHvEmulator::WHvEmulator()
	: WHvEmulator(nullptr)
{}

WHvEmulator::~WHvEmulator() noexcept(false) {
	if (nullptr != m_Handle) {
		WIN32_ERROR_ASSERT(::WHvEmulatorDestroyEmulator(m_Handle));
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
}

auto WHvEmulator::IoPortAccess(void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* info_v) -> HRESULT
{
}

auto WHvEmulator::GetRegisters(void* context_v, const WHV_REGISTER_NAME* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v)
{
}

auto WHvEmulator::SetRegisters(void* context_v, const WHV_REGISTER_NAME* names_v, uint32_t count_v, const WHV_REGISTER_VALUE* values_v)
{
}

auto WHvEmulator::TranslateGvaPage(void* context_v, uint64_t gva_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE* result_v, WHV_GPA* gpa_v) -> HRESULT
{
}
