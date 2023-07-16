#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>


namespace win32
{
	struct WHvEmulator	{

		WHvEmulator();
		~WHvEmulator();

		WHvEmulator(WHvEmulator&& other_v) noexcept;
		auto operator=(WHvEmulator&& other_v) noexcept -> WHvEmulator&;

		WHvEmulator(WHvEmulator const&) = delete;
		auto operator=(WHvEmulator const&) -> WHvEmulator& = delete;

		auto swap(WHvEmulator& other_v) noexcept -> void;

		auto GetHandle() const noexcept -> WHV_EMULATOR_HANDLE;

		static auto Create() -> WHvEmulator;
		
	protected:
		WHvEmulator(WHV_EMULATOR_HANDLE handle_v) noexcept;

		auto IoPortAccess(void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* access_v) -> void;
		auto MemoryAccess(void* context_v, WHV_EMULATOR_MEMORY_ACCESS_INFO* access_v) -> void;
		auto GetRegisters(void* context_v, WHV_REGISTER_NAME const* register_names_v, uint32_t register_count_v, WHV_REGISTER_VALUE* register_values_v) -> void;
		auto SetRegisters(void* context_v, WHV_REGISTER_NAME const* register_names_v, uint32_t register_count_v, WHV_REGISTER_VALUE const* register_values_v) -> void;
		auto TranslateGvaPage(void* context_v, uint64_t gva_v, WHV_TRANSLATE_GVA_FLAGS translate_flags_v, WHV_TRANSLATE_GVA_RESULT_CODE* result_code_v, uint64_t* gpa_v) -> void;


	private:
		WHV_EMULATOR_HANDLE m_Handle{ nullptr };			
	};

}

