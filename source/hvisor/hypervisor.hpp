#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinHvPlatform.h>
#include <WinHvPlatformDefs.h>

#include <cstdint>
#include <cstddef>
#include <span>

struct HyperVisor
{
	static inline constexpr auto all_permissions = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute;

	HyperVisor ();
	~HyperVisor ();

	auto MapPhysical(std::span<std::byte> source_v, std::uint64_t destination_v, WHV_MAP_GPA_RANGE_FLAGS flags_v = all_permissions) -> void;

private:
	WHV_PARTITION_HANDLE m_partition;
};