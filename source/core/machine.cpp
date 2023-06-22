#include <win32/error.hpp>

#include <stdexcept>

#include "machine.hpp"

using core::Machine;

struct RealmodeInitState
{
	static constexpr const WHV_REGISTER_NAME Names[] = {
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
	
	static constexpr const WHV_REGISTER_VALUE Values[] = 
	{
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

	static constexpr const std::size_t Count = std::min(std::size(Names), std::size(Values));
};


Machine::Machine(Config const& config_v)
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Partition));
	InitializePartitionProperties();
	config_v.ApplyBeforeSetup(*this);
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_Partition));
	config_v.ApplyAfterSetup(*this);
}

Machine::~Machine()
{
	if (m_Partition) {
		WHvDeletePartition(m_Partition);
		m_Partition = nullptr;
	}
	m_Memories.clear();
}

auto Machine::MapMemory(std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t flags_v, std::uint64_t offset_v) -> void 
{
	if (index_v >= m_Memories.size()) {
		throw std::invalid_argument("Invalid memory index");
	}

	auto address_v = m_Memories[index_v].Data() + offset_v;

	if (0u == size_v) {
		if (offset_v >= m_Memories[index_v].Size()) {
			throw std::invalid_argument("Invalid memory offset");
		}
		size_v = m_Memories[index_v].Size() - offset_v;	
	}

	WIN32_ERROR_ASSERT(::WHvMapGpaRange(m_Partition, address_v, base_v, size_v, (WHV_MAP_GPA_RANGE_FLAGS)flags_v));
}

void Machine::UnmapMemory(std::uint64_t base_v, std::uint64_t size_v)
{
	WIN32_ERROR_ASSERT(::WHvUnmapGpaRange(m_Partition, base_v, size_v));
}

auto Machine::InitializeProcessor(std::uint32_t index_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_Partition, index_v, 0u));
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v, 
		RealmodeInitState::Names, RealmodeInitState::Count, RealmodeInitState::Values));
}

auto Machine::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Partition, code_v, data_v, size_v));
}

auto Machine::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Partition, code_v, data_v, size_v, &size_v));
}

auto Machine::InitializePartitionProperties() -> void {
	SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, std::uint64_t{ 0 });
	SetProperty(WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS { .HypercallExit = 1 });
	SetProperty(WHvPartitionPropertyCodeProcessorFeatures, WHV_PROCESSOR_FEATURES { .LahfSahfSupport = 1 });
}