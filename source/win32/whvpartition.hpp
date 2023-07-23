#pragma once 

#include <cstddef>
#include <cstdint>
#include <cassert>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <utils/objects.hpp>
#include <utils/span.hpp>
#include <core/accessflags.hpp>

namespace win32
{

	struct WHvPartition 
	{	
		static auto Create() -> WHV_PARTITION_HANDLE;
	
		WHvPartition();
		WHvPartition(std::initializer_list<std::pair<WHV_PARTITION_PROPERTY_CODE, WHV_PARTITION_PROPERTY>> props_v);
		~WHvPartition() noexcept(false);
	
		WHvPartition(WHvPartition&& other_v) noexcept;
		auto operator=(WHvPartition&& other_v) noexcept -> WHvPartition&;
	
		WHvPartition(WHvPartition const&) = delete;
		auto operator=(WHvPartition const&) -> WHvPartition& = delete;

		auto swap(WHvPartition& other_v) noexcept -> void;
	
		auto GetHandle() const noexcept -> WHV_PARTITION_HANDLE;
		auto Setup () const -> HRESULT;
		auto Reset () const -> HRESULT;
	
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, std::span<std::byte const> value_v) const -> HRESULT;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, std::span<std::byte>& span_v) const -> HRESULT;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, std::span<std::byte>&& span_v) const -> HRESULT {
			return GetProperty(property_v, span_v);
		}

		template <typename T>
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, T const& value_v) const -> HRESULT
		{
			return SetProperty(property_v, utils::as_bytes(value_v));
		}

		template <typename T>
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, T& value_v) const -> HRESULT
		{
			return GetProperty(property_v, utils::as_bytes(value_v));
		}

		auto MapGpaRange(void* buffer_v, std::uint64_t physaddr_v, std::uint64_t length_v, core::Access flags_v) const-> HRESULT;
		auto UnmapGpaRange(std::uint64_t physaddr_v, std::uint64_t length_v) const-> HRESULT;

	protected:
		WHvPartition(WHV_PARTITION_HANDLE handle_v) noexcept;
	
	private:
		WHV_PARTITION_HANDLE m_Handle{ nullptr };
	};

}