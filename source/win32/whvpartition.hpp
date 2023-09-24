#pragma once 

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <ranges>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <utils/span.hpp>
#include <core/accessflags.hpp>

namespace win32
{

	struct WHvPartition 
	{
		using property_pair = std::pair<WHV_PARTITION_PROPERTY_CODE, WHV_PARTITION_PROPERTY>;

		WHvPartition (WHV_PARTITION_HANDLE handle_v) noexcept;
		~WHvPartition (); 

		auto swap (WHvPartition& with_v) noexcept -> void;
		WHvPartition (WHvPartition&& from_v) noexcept;
		auto operator = (WHvPartition&& from_v) noexcept -> WHvPartition&;

		WHvPartition (WHvPartition const& other_v) = delete;
		auto operator = (WHvPartition const& other_v) -> WHvPartition& = delete;

		static auto Create(std::uint32_t vcpucount_v, std::span<property_pair const> props_v) -> WHV_PARTITION_HANDLE; 

		static auto Create(std::uint32_t vcpucount_v, std::initializer_list<property_pair const> props_v) -> WHV_PARTITION_HANDLE {
			return Create(vcpucount_v, std::span{ props_v });
		}

		auto GetHandle() const -> WHV_PARTITION_HANDLE;
		auto Reset() const -> std::int32_t;
		auto MapGpaRange(void*, std::uint64_t, std::uint64_t, core::Access) const -> std::int32_t;
		auto MapGpaRange(void const*, std::uint64_t, std::uint64_t, core::Access) const->std::int32_t;
		auto UnmapGpaRange(std::uint64_t, std::uint64_t) const -> std::int32_t;
		auto QueryGpaRangeDirtyBitmap(std::uint64_t address_v, std::uint64_t size_v, std::span<std::uint64_t> bitmap_v) const -> std::int32_t;
		auto ClearGpaRangeDirtyBitmap(std::uint64_t address_v, std::uint64_t size_v) const -> std::int32_t;

	protected:
		static auto GetProcessorCount(WHV_PARTITION_HANDLE handle_v) -> std::tuple<std::int32_t, std::uint32_t>;
		static auto SetProcessorCount(WHV_PARTITION_HANDLE handle_v, std::uint32_t count_v) -> std::int32_t;

		static auto SetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte const> value_v) -> std::int32_t;
		static auto GetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte>& value_v) -> std::int32_t;
		static auto GetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte>&& value_v) -> std::int32_t {
			return GetProperty(handle_v, code_v, value_v);
		}

		template <typename T> requires (std::is_trivial_v<T>)
		static auto SetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, T const& value_v) -> std::int32_t {
			return SetProperty(handle_v, code_v, utils::as_bytes(value_v));
		}

		template <typename T> requires (std::is_trivial_v<T>)
		static auto GetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, T& value_v) -> std::int32_t {
			return GetProperty(handle_v, code_v, utils::as_mutable_bytes(value_v));
		}

		static auto SetProperties(WHV_PARTITION_HANDLE handle_v, std::span<property_pair const> props_v) -> std::int32_t;

	public:
		template <typename... T> requires
		requires (WHV_PARTITION_HANDLE m_handle, WHV_PARTITION_PROPERTY_CODE code_v, T&&... args_v) {
		{ WHvPartition::SetProperty(m_handle, code_v, std::forward<T>(args_v)...) } -> std::same_as<std::int32_t>; }
		inline auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T&&... args_v) const -> std::int32_t {
			return WHvPartition::SetProperty(m_handle, code_v, std::forward<T>(args_v)...);
		}

		template <typename... T> requires
		requires (WHV_PARTITION_HANDLE m_handle, WHV_PARTITION_PROPERTY_CODE code_v, T&&... args_v) {
		{  WHvPartition::GetProperty(m_handle, code_v, std::forward<T>(args_v)...) } -> std::same_as<std::int32_t>; }
		inline auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T&&... args_v) const -> std::int32_t {
			return WHvPartition::GetProperty(m_handle, code_v, std::forward<T>(args_v)...);
		}

		template <typename T>
		inline auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v) const -> T {
			T value{}; WIN32_ERROR_ASSERT(GetProperty(code_v, value));
			return value;
		}

		auto IsMapped(std::uint64_t base_v) const -> bool;

	protected:
		auto Mark(std::uint64_t base_v, std::uint64_t size_v, bool is_mapped_v = false) const -> void;

	private:
		WHV_PARTITION_HANDLE m_handle;
		mutable std::vector<bool> m_IsMapped;
	};

}