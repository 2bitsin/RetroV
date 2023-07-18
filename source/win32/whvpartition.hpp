#pragma once 

#include <cstddef>
#include <cstdint>
#include <cassert>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/objects.hpp>

namespace win32
{

	struct WHvPartition 
	{	
		static auto Create() -> WHvPartition;
	
		WHvPartition();
		~WHvPartition();
	
		WHvPartition(WHvPartition&& other_v) noexcept;
		auto operator=(WHvPartition&& other_v) noexcept -> WHvPartition&;
	
		WHvPartition(WHvPartition const&) = delete;
		auto operator=(WHvPartition const&) -> WHvPartition& = delete;

		auto swap(WHvPartition& other_v) noexcept -> void;
	
		auto GetHandle() const noexcept -> WHV_PARTITION_HANDLE;
		auto Setup () const -> HRESULT;
		auto Reset () const -> HRESULT;
	
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, void const* buffer_v, uint32_t size_v) const -> HRESULT;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, void* buffer_v, uint32_t& size_v) const -> HRESULT;

		template <typename T>
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, T const& value_v) const -> HRESULT
		{
			return SetProperty(property_v, &value_v, (uint32_t)sizeof(T));
		}

		template <typename T>
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, T& value_v) const -> HRESULT
		{
			uint32_t size_v{ sizeof(T) };
			auto result_v = GetProperty(property_v, &value_v, size_v);
			assert(size_v == sizeof(T));
			return result_v;
		}

		auto MapGpaRange(void* buffer_v, std::uint64_t physaddr_v, std::uint64_t length_v, WHV_MAP_GPA_RANGE_FLAGS flags_v) const-> HRESULT;
		auto UnmapGpaRange(std::uint64_t physaddr_v, std::uint64_t length_v) const-> HRESULT;

	protected:
		WHvPartition(WHV_PARTITION_HANDLE handle_v) noexcept;
	
	private:
		WHV_PARTITION_HANDLE m_Handle{ nullptr };
	};

}