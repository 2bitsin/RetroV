#include <win32/whvemulator.hpp>

#include <cassert>

#include <utility>
using std::exchange;

using win32::WHvEmulator;

WHvEmulator::WHvEmulator(WHV_EMULATOR_HANDLE handle_v) noexcept
	: m_handle{ handle_v }
{}

auto win32::WHvEmulator::TryIoEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_X64_IO_PORT_ACCESS_CONTEXT const& ioctx_v) const noexcept 
	-> std::tuple<std::int32_t, WHV_EMULATOR_STATUS>
{
	WHV_EMULATOR_STATUS status_v;
  auto result_v = WHvEmulatorTryIoEmulation(m_handle, context_v, &vpctx_v, &ioctx_v, &status_v);
	return{ result_v, status_v };
}

auto win32::WHvEmulator::TryMmioEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_MEMORY_ACCESS_CONTEXT const& mmctx_v) const noexcept 
	-> std::tuple<std::int32_t, WHV_EMULATOR_STATUS>
{
  WHV_EMULATOR_STATUS status_v;
	auto result_v = WHvEmulatorTryMmioEmulation(m_handle, context_v, &vpctx_v, &mmctx_v, &status_v);
	return{ result_v, status_v };
}

WHvEmulator::WHvEmulator()
	: WHvEmulator(nullptr)
{}

WHvEmulator::~WHvEmulator() noexcept(false) {
	if (nullptr != m_handle) {
		WIN32_ERROR_ASSERT(::WHvEmulatorDestroyEmulator(m_handle));
	}
}

WHvEmulator::WHvEmulator(WHvEmulator&& other_v) noexcept
	: m_handle{ exchange(other_v.m_handle, nullptr) }
{}

auto WHvEmulator::operator=(WHvEmulator&& other_v) noexcept -> WHvEmulator& {
	if (this != &other_v) {
		auto temp_v{ std::move(other_v) };
		temp_v.swap(*this);
	}
	return *this;	
}

auto WHvEmulator::swap(WHvEmulator& other_v) noexcept -> void {
	std::swap(m_handle, other_v.m_handle);
}

auto win32::WHvEmulator::Create() -> WHV_EMULATOR_HANDLE {
  return Create({
		sizeof(WHV_EMULATOR_CALLBACKS), 0u,
		&IoPortAccess,
		&MemoryAccess,
		&GetRegisters,
		&SetRegisters,
		&TranslateGvaPage
	});
}

auto WHvEmulator::GetHandle() const noexcept -> WHV_EMULATOR_HANDLE {
	return m_handle;
}

auto WHvEmulator::SetHandle(WHV_EMULATOR_HANDLE handle_v) noexcept -> void
{
	WHvEmulator::~WHvEmulator();
	m_handle = handle_v;
}

auto WHvEmulator::Create(WHV_EMULATOR_CALLBACKS const& callbacks_v) -> WHV_EMULATOR_HANDLE {
	WHV_EMULATOR_HANDLE handle_v{ nullptr };
	WIN32_ERROR_ASSERT(::WHvEmulatorCreateEmulator(&callbacks_v, &handle_v));
	return handle_v;
}

auto WHvEmulator::MemoryAccess(void* context_v, WHV_EMULATOR_MEMORY_ACCESS_INFO* info_v) -> HRESULT {	
	auto const& context_r = *(InvocationContext*)context_v;
	assert (nullptr != context_r.MemoryAccess);
	return context_r.MemoryAccess(context_r.ObjectPointer, info_v);
}

auto WHvEmulator::IoPortAccess(void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* info_v) -> HRESULT {
	auto const& context_r = *(InvocationContext*)context_v;
	assert (nullptr != context_r.IoPortAccess);
	return context_r.IoPortAccess(context_r.ObjectPointer, info_v);
}

auto WHvEmulator::GetRegisters(void* context_v, 
	const WHV_REGISTER_NAME* names_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT
{
	auto const& context_r = *(InvocationContext*)context_v;
	assert (nullptr != context_r.GetRegisters);
	return context_r.GetRegisters(context_r.ObjectPointer, names_v, count_v, values_v);
}

auto WHvEmulator::SetRegisters(void* context_v, 
	const WHV_REGISTER_NAME* names_v, uint32_t count_v, const WHV_REGISTER_VALUE* values_v) -> HRESULT
{
	auto const& context_r = *(InvocationContext*)context_v;
	assert (nullptr != context_r.SetRegisters);
	return context_r.SetRegisters(context_r.ObjectPointer, names_v, count_v, values_v);
}

auto WHvEmulator::TranslateGvaPage(void* context_v, WHV_GUEST_VIRTUAL_ADDRESS virtaddr_v, 
	WHV_TRANSLATE_GVA_FLAGS falgs_v, WHV_TRANSLATE_GVA_RESULT_CODE* code_v, WHV_GUEST_PHYSICAL_ADDRESS* physaddr_v) -> HRESULT
{
	auto const& context_r = *(InvocationContext*)context_v;
	assert (nullptr != context_r.TranslateGvaPage);
	return context_r.TranslateGvaPage(context_r.ObjectPointer, virtaddr_v, falgs_v, code_v, physaddr_v);
}
