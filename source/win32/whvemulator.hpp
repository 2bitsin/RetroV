#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utils/span.hpp>

#include <type_traits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>	

namespace win32
{
	namespace detail
	{

		template <typename ObjectT>
		concept Has_IoPortAccess = requires(ObjectT&& object_v, 
			bool is_write_v, std::uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v)
		{		
			{ object_v.IoPortAccess(is_write_v, addr_v, data_v) } -> std::same_as<std::int32_t>;
		};

		template <typename ObjectT>
		concept Has_MemoryAccess = requires(ObjectT&& object_v, 
			bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v)
		{		
			{ object_v.MemoryAccess(is_write_v, addr_v, data_v) } -> std::same_as<std::int32_t>;
		};

		template <typename ObjectT>
		concept Has_GetRegisters = requires(ObjectT&& object_v, std::span<WHV_REGISTER_NAME const> names_v, 
			std::span<WHV_REGISTER_VALUE> values_v)
		{		
			{ object_v.GetRegisters(names_v, values_v) } -> std::same_as<std::int32_t>;
		};

		template <typename ObjectT>
		concept Has_SetRegisters = requires(ObjectT&& object_v, std::span<WHV_REGISTER_NAME const> names_v, 
			std::span<WHV_REGISTER_VALUE const> values_v)
		{		
			{ object_v.SetRegisters(names_v, values_v) } -> std::same_as<std::int32_t>;
		};

		template <typename ObjectT>
		concept Has_TranslateGvaPage = requires(ObjectT&& object_v, std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, 
			WHV_TRANSLATE_GVA_RESULT_CODE& code_v, std::uint64_t& physaddr_v)
		{		
			{ object_v.TranslateGvaPage(virtaddr_v, flags_v, code_v, physaddr_v) } -> std::same_as<std::int32_t>;
		};

	}

	struct WHvEmulator
	{
		static auto Create(WHV_EMULATOR_CALLBACKS const& callbacks_v) -> WHV_EMULATOR_HANDLE;
		static auto Create() -> WHV_EMULATOR_HANDLE;

		WHvEmulator();
		WHvEmulator(WHV_EMULATOR_HANDLE handle_v) noexcept;
		~WHvEmulator() noexcept(false);

		WHvEmulator(WHvEmulator&& other_v) noexcept;
		auto operator=(WHvEmulator&& other_v) noexcept -> WHvEmulator&;
		WHvEmulator(WHvEmulator const&) = delete;
		auto operator=(WHvEmulator const&) -> WHvEmulator& = delete;		
		
		auto swap(WHvEmulator& other_v) noexcept -> void;

		auto GetHandle() const noexcept -> WHV_EMULATOR_HANDLE;

		template <typename ObjectT>
		auto TryIoEmulation(ObjectT& object_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_X64_IO_PORT_ACCESS_CONTEXT const& ioctx_v) const noexcept 
			-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>
		{
			InvocationContext callbacks_v;
			MakeInvocationContext(object_v, callbacks_v);
			return TryIoEmulation(std::addressof(callbacks_v), vpctx_v, ioctx_v);
		}

		template <typename ObjectT>
		auto TryMmioEmulation(ObjectT& object_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_MEMORY_ACCESS_CONTEXT const& mmctx_v) const noexcept 
			-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>
		{
			InvocationContext callbacks_v;
			MakeInvocationContext(object_v, callbacks_v);
			return TryMmioEmulation(std::addressof(callbacks_v), vpctx_v, mmctx_v);
		}
		
	protected:

		auto TryIoEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_X64_IO_PORT_ACCESS_CONTEXT const& ioctx_v) const noexcept 
			-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>;
		auto TryMmioEmulation(void* context_v, WHV_VP_EXIT_CONTEXT const& vpctx_v, WHV_MEMORY_ACCESS_CONTEXT const& mmctx_v) const noexcept 
			-> std::tuple<HRESULT, WHV_EMULATOR_STATUS>;

		struct InvocationContext
		{
			void* ObjectPointer;
			WHV_EMULATOR_IO_PORT_CALLBACK IoPortAccess;
			WHV_EMULATOR_MEMORY_CALLBACK MemoryAccess;
			WHV_EMULATOR_GET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK GetRegisters;
			WHV_EMULATOR_SET_VIRTUAL_PROCESSOR_REGISTERS_CALLBACK SetRegisters;
			WHV_EMULATOR_TRANSLATE_GVA_PAGE_CALLBACK TranslateGvaPage;
		};

		template <typename T>
		static auto MakeInvocationContext(T& object_v, InvocationContext& callbacks_v) noexcept -> void 
		{
			callbacks_v.ObjectPointer = (void*)std::addressof(object_v);
			callbacks_v.IoPortAccess = nullptr;
			static_assert(detail::Has_IoPortAccess<T>);
			if constexpr (detail::Has_IoPortAccess<T>) {
				callbacks_v.IoPortAccess = [](void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* access_v) -> HRESULT {
					if (context_v == nullptr)
						return E_INVALIDARG;
					return static_cast<T*>(context_v)->IoPortAccess(
						(bool)access_v->Direction, 
						(std::uint16_t)access_v->Port,						
						utils::as_static_mutable_bytes(access_v->Data)
							.first(access_v->AccessSize));
				};
			}

			callbacks_v.MemoryAccess = nullptr;
			static_assert(detail::Has_MemoryAccess<T>);
			if constexpr (detail::Has_MemoryAccess<T>) {
				callbacks_v.MemoryAccess = [](void* context_v, WHV_EMULATOR_MEMORY_ACCESS_INFO* access_v) -> HRESULT {
					if (context_v == nullptr)
						return E_INVALIDARG;
					return static_cast<T*>(context_v)->MemoryAccess(
						(bool)access_v->Direction,
						(std::uint64_t)access_v->GpaAddress,
						utils::as_static_mutable_bytes(access_v->Data)
							.first(access_v->AccessSize));
				};
			}

			callbacks_v.GetRegisters = nullptr;
			static_assert(detail::Has_GetRegisters<T>);
			if constexpr (detail::Has_GetRegisters<T>) {
				callbacks_v.GetRegisters = [](void* context_v, WHV_REGISTER_NAME const* rnames_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT {
					if (context_v == nullptr)
						return E_INVALIDARG;
					return static_cast<T*>(context_v)->GetRegisters(
						{ rnames_v, count_v },
						{ values_v, count_v });
				};
			}

			callbacks_v.SetRegisters = nullptr;
			static_assert(detail::Has_SetRegisters<T>);
			if constexpr (detail::Has_SetRegisters<T>) {
				callbacks_v.SetRegisters = [](void* context_v, WHV_REGISTER_NAME const* rnames_v, uint32_t count_v, 
					WHV_REGISTER_VALUE const* values_v) -> HRESULT
				{
					if (context_v == nullptr)
						return E_INVALIDARG;
					return static_cast<T*>(context_v)->SetRegisters(
						{ rnames_v, count_v },
						{ values_v, count_v });
				};
			}

			callbacks_v.TranslateGvaPage = nullptr;
			static_assert(detail::Has_TranslateGvaPage<T>);
			if constexpr (detail::Has_TranslateGvaPage<T>) {
				callbacks_v.TranslateGvaPage = [](void* context_v, uint64_t gva_v, WHV_TRANSLATE_GVA_FLAGS flags_v, 
					WHV_TRANSLATE_GVA_RESULT_CODE* code_v, uint64_t* gpa_v) -> HRESULT
				{
					if (context_v == nullptr)
						return E_INVALIDARG;
					return static_cast<T*>(context_v)->TranslateGvaPage(
						gva_v, flags_v, *code_v, *gpa_v);
				};
			}			
		}


		static auto __stdcall IoPortAccess(void* context_v, WHV_EMULATOR_IO_ACCESS_INFO* access_v) -> HRESULT;
		static auto __stdcall MemoryAccess(void* context_v, WHV_EMULATOR_MEMORY_ACCESS_INFO* access_v) -> HRESULT;
		static auto __stdcall GetRegisters(void* context_v, WHV_REGISTER_NAME const* rnames_v, uint32_t count_v, WHV_REGISTER_VALUE* values_v) -> HRESULT;
		static auto __stdcall SetRegisters(void* context_v, WHV_REGISTER_NAME const* rnames_v, uint32_t count_v, WHV_REGISTER_VALUE const* values_v) -> HRESULT;
		static auto __stdcall TranslateGvaPage(void* context_v, WHV_GUEST_VIRTUAL_ADDRESS virtaddr_v, WHV_TRANSLATE_GVA_FLAGS falgs_v, WHV_TRANSLATE_GVA_RESULT_CODE* code_v, WHV_GUEST_PHYSICAL_ADDRESS* physaddr_v) -> HRESULT;

	private:
		WHV_EMULATOR_HANDLE m_Handle{ nullptr };			
	};

}

