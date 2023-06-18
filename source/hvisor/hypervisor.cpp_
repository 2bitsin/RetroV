#include <win32/Hresult.hpp>
#include <hvisor/hypervisor.hpp>

static constexpr const WHV_REGISTER_NAME G_register_names[] = {
	/*  0 */ WHvX64RegisterRflags,
	/*  1 */ WHvX64RegisterRip,
	/*  2 */ WHvX64RegisterCs,
	/*  3 */ WHvX64RegisterDs,
	/*  4 */ WHvX64RegisterEs,
	/*  5 */ WHvX64RegisterSs,
	/*  6 */ WHvX64RegisterFs,
	/*  7 */ WHvX64RegisterGs,
	/*  8 */ WHvX64RegisterIdtr,
	/*  9 */ WHvX64RegisterGdtr,
	/*  A */ WHvX64RegisterRax,
	/*  B */ WHvX64RegisterRbx,
	/*  C */ WHvX64RegisterRcx,
	/*  D */ WHvX64RegisterRdx,
	/*  E */ WHvX64RegisterRsi,
	/*  F */ WHvX64RegisterRdi,
	/* 10 */ WHvX64RegisterRbp,
	/* 11 */ WHvX64RegisterRsp,
	/* 12 */ WHvX64RegisterR8,
	/* 13 */ WHvX64RegisterR9,
	/* 14 */ WHvX64RegisterR10,
	/* 15 */ WHvX64RegisterR11,
	/* 16 */ WHvX64RegisterR12,
	/* 17 */ WHvX64RegisterR13,
	/* 18 */ WHvX64RegisterR14,
	/* 19 */ WHvX64RegisterR15,
	/* 1A */ WHvX64RegisterCr0,
};

WHV_REGISTER_VALUE G_real_mode_values [] = {
	{.Reg64 = 0x0000000000000002u },
	{.Reg64 = 0x000000000000FFF0u },

	{.Segment = {.Base = 0xf0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
	{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },

	{.Table = {.Limit = 0x03FFu, .Base = 0x00000000u  } },
	{.Table = {.Limit = 0x0000u, .Base = 0x00000000u  } },

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},
	{.Reg64 = 0x0000000000000000u},

	{.Reg64 = 0x0000000000000010u}
};

static_assert(std::size(G_real_mode_values) == std::size(G_register_names));

static inline auto WHvSetPartitionProperty_(auto&& partition_v, auto&& code_v, auto const& data_v) {
	return WHvSetPartitionProperty(partition_v, code_v, &data_v, sizeof(data_v));
}

static inline auto assert_(auto result_v) -> void {
	if (result_v != S_OK) throw win32_error(result_v);
}

Hypervisor::Hypervisor() 	
	:	m_partition { nullptr }
{
	static constexpr auto every_permission_s = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute;

	

	assert_(WHvCreatePartition(&m_partition));
	assert_(WHvSetPartitionProperty_(m_partition, WHvPartitionPropertyCodeProcessorCount, std::uint32_t{ 1u }));
	assert_(WHvSetPartitionProperty_(m_partition, WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .HypercallExit = 1 }));
	assert_(WHvSetupPartition(m_partition));
	assert_(WHvCreateVirtualProcessor(m_partition, 0, 0));
}

Hypervisor::~Hypervisor() 
{
	if (!m_partition) return;
	WHvDeleteVirtualProcessor(m_partition, 0u);
	WHvDeletePartition(m_partition);
}

auto Hypervisor::MapPhysical(std::span<std::byte> source_v, std::uint64_t destination_v, std::uint32_t access_v) -> void 
{
	WHV_MAP_GPA_RANGE_FLAGS flags_v{ WHvMapGpaRangeFlagNone };
	if ( access_v & AccessRead    ) flags_v |= WHvMapGpaRangeFlagRead;
	if ( access_v & AccessWrite   ) flags_v |= WHvMapGpaRangeFlagWrite;
	if ( access_v & AccessExecute ) flags_v |= WHvMapGpaRangeFlagExecute;
	if ( access_v & TrackDirty    ) flags_v |= WHvMapGpaRangeFlagTrackDirtyPages;
	assert_(WHvMapGpaRange(m_partition, source_v.data(), destination_v, source_v.size(), flags_v));
}

auto Hypervisor::UnmapPhysical(std::uint64_t destination_v, std::uint64_t size_v) -> void
{
	assert_(WHvUnmapGpaRange(m_partition, destination_v, size_v));
}

auto Hypervisor::RestartToRealMode() -> void
{
	static constexpr auto length_s = std::min(std::size(G_real_mode_values), std::size(G_register_names));
	assert_(WHvSetVirtualProcessorRegisters(m_partition, 0, G_register_names, length_s, G_real_mode_values));
}

auto Hypervisor::Run() -> WHV_RUN_VP_EXIT_CONTEXT
{	
	WHV_RUN_VP_EXIT_CONTEXT exit_v;
	assert_(WHvRunVirtualProcessor(m_partition, 0, &exit_v, sizeof(exit_v)));
	WHV_REGISTER_VALUE registers_v[std::size(G_register_names)];
	WHvGetVirtualProcessorRegisters(m_partition, 0, G_register_names, std::size(G_register_names), registers_v);
	return exit_v;
}

auto Hypervisor::SetRegister(WHV_REGISTER_NAME name_v, WHV_REGISTER_VALUE const& value) -> void
{
	assert_(WHvSetVirtualProcessorRegisters(m_partition, 0, &name_v, 1, &value));
}
