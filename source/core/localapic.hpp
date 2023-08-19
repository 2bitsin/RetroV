#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <tuple>

#include <win32/winhvpx.hpp>

namespace core
{
	struct Machine;
	struct Processor;

	struct LocalApic
	{
		static inline constexpr const std::uint16_t EOIR = 0xBu;
		static inline constexpr const std::uint16_t SIVR = 0xFu;

		static inline constexpr const std::uint32_t SIVR_APIC_ENABLED = 0x100u;

		LocalApic(Machine& machine_v, std::uint32_t vcpu_index_v);

		auto Initialize() -> std::int32_t;

		auto XApicWrite(std::uint16_t register_v, std::uint32_t value_v) const -> std::int32_t;
		auto XApicFetch(std::uint16_t register_v) const -> std::tuple<std::int32_t, std::uint32_t>;
		
		auto GetBase() const -> std::tuple<std::int32_t, std::uint64_t, bool>;
		auto SetBase(std::uint64_t base_v, bool enable_v) const->std::int32_t;

		auto SignalEOI() const -> std::int32_t;

		auto RequestIRQ(
			WHV_INTERRUPT_TRIGGER_MODE mode_v = WHvX64InterruptTriggerModeEdge,
			WHV_INTERRUPT_TYPE type_v = WHvX64InterruptTypeFixed,
			std::uint8_t vector_v = 0u,
			WHV_INTERRUPT_DESTINATION_MODE dest_v = WHvX64InterruptDestinationModeLogical
			) const -> std::int32_t;

	private:
		std::vector<std::byte> m_InitState;
		Machine& m_Machine;
		Processor& m_Processor;
	};

}