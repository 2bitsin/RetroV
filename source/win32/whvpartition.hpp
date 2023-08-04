#pragma once 

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <ranges>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <utils/objects.hpp>
#include <utils/span.hpp>
#include <core/accessflags.hpp>

namespace win32
{

	struct WHvPartition 
	{
		using property_pair = std::pair<WHV_PARTITION_PROPERTY_CODE, WHV_PARTITION_PROPERTY>;

		WHvPartition (WHV_PARTITION_HANDLE handle_v) noexcept;
		~WHvPartition (); 

		static auto Create(std::uint32_t vcpucount_v, std::initializer_list<property_pair const> props_v) -> WHV_PARTITION_HANDLE;

		auto GetHandle() const -> WHV_PARTITION_HANDLE;

		auto Reset() const -> std::int32_t;


		auto MapGpaRange(void*, std::uint64_t, std::uint64_t, core::Access) const -> std::int32_t;
		auto UnmapGpaRange(std::uint64_t, std::uint64_t) const -> std::int32_t;


	};

}