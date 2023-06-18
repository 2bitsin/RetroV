#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinHvPlatform.h>
#include <WinHvPlatformDefs.h>

#include <cstdint>
#include <cstddef>
#include <span>

struct Hypervisor
{
	static inline constexpr auto AccessRead    = 0x01u;
	static inline constexpr auto AccessWrite   = 0x02u;
	static inline constexpr auto AccessExecute = 0x04u;
	static inline constexpr auto TrackDirty    = 0x08u;
	static inline constexpr auto AccessAll     = AccessRead|AccessWrite|AccessExecute;
	static inline constexpr auto AccessRom		 = AccessRead|AccessExecute;
	static inline constexpr auto AccessDev     = AccessRead|AccessWrite;

	Hypervisor ();
	~Hypervisor ();

	auto MapPhysical(std::span<std::byte> source_v, std::uint64_t destination_v, std::uint32_t access_v = AccessAll) -> void;
	auto UnmapPhysical(std::uint64_t destination_v, std::uint64_t size_v) -> void;
	auto RestartToRealMode() -> void;
	auto Run() -> WHV_RUN_VP_EXIT_CONTEXT;
	auto SetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE const& value) -> void;

private:
	WHV_PARTITION_HANDLE m_partition;
};