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
		static inline constexpr const std::uint16_t EOIR		= 0x0Bu;	// End of Interrupt Register
		static inline constexpr const std::uint16_t SIVR		= 0x0Fu;	// Spurious Interrupt Vector Register
		static inline constexpr const std::uint16_t ESR			= 0x28u;	// Error Status Register
		static inline constexpr const std::uint16_t LVTCMCI	= 0x2Fu;	// LVT Corrected Machine Check Interrupt
		static inline constexpr const std::uint16_t LVTICR0	= 0x30u;	// LVT Interrupt Command Register 0
		static inline constexpr const std::uint16_t LVTICR1	= 0x31u;	// LVT Interrupt Command Register	1
		static inline constexpr const std::uint16_t LVTTR		= 0x32u;	// LVT Timer Register
		static inline constexpr const std::uint16_t LVTTSR	= 0x33u;	// LVT Thermal Sensor Register
		static inline constexpr const std::uint16_t LVTPCR	= 0x34u;	// LVT Performance Counter Register
		static inline constexpr const std::uint16_t LVTL0		= 0x35u;	// LVT LINT0 Register
		static inline constexpr const std::uint16_t LVTL1		= 0x36u;	// LVT LINT1 Register
		static inline constexpr const std::uint16_t LVTERR	= 0x37u;	// LVT Error Register

		static inline constexpr const std::uint32_t SIVR_APIC_ENABLED = 0x100u;
		static inline constexpr const std::uint32_t APIC_BASE_ENABLED = 0x800u;

		enum eDeliveryMode: ::std::uint32_t {
			Fixed						= 0b000,
			LowestPriority	= 0b001,
			SMI							= 0b010,
			Reserved_ 			= 0b011,
			NMI							= 0b100,
			INIT						= 0b101,
			SIPI						= 0b110,
			ExtINT					= 0b111
		};

		enum class eTriggerMode : ::std::uint32_t {
			Edge	= 0b0,
			Level	= 0b1
		};

		enum class eMaskingMode : ::std::uint32_t {
			NotMasked	= 0b0,
			Masked		= 0b1
		};

		enum class ePinPolarity : ::std::uint32_t {
			ActiveHigh	= 0b0,
			ActiveLow		= 0b1
		};

	#pragma pack(push, 1)
		union LvtEntry
		{
			struct
			{
				std::uint32_t Vector:8;
				eDeliveryMode DeliveryMode:3;
				std::uint32_t Reserved1:1;
				std::uint32_t DeliveryStatus:1;
				ePinPolarity	PinPolarity:1;
				std::uint32_t	RemoteIRR:1;
				eTriggerMode  TriggerMode:1;
				eMaskingMode  Mask:1;
				std::uint32_t Reserved2:15;
			};
			std::uint32_t AsUint32;

			inline operator std::uint32_t() const
			{
				return AsUint32;
			}
		};
	#pragma pack(pop)

		LocalApic(Machine& machine_v, std::uint32_t vcpu_index_v);

		auto Initialize() -> std::int32_t;

		auto XApicWrite(std::uint16_t register_v, std::uint32_t value_v) const -> std::int32_t;
		auto XApicFetch(std::uint16_t register_v) const -> std::tuple<std::int32_t, std::uint32_t>;
		
		auto GetBase() const -> std::tuple<std::int32_t, std::uint64_t, bool>;
		auto SetBase(std::uint64_t base_v, bool enable_v) const->std::int32_t;

		auto SignalEOI() const -> std::int32_t;

		auto RequestIRQ(WHV_INTERRUPT_TRIGGER_MODE mode_v = WHvX64InterruptTriggerModeEdge, 
			WHV_INTERRUPT_TYPE type_v = WHvX64InterruptTypeFixed, std::uint8_t vector_v = 0u, 
			WHV_INTERRUPT_DESTINATION_MODE dest_v = WHvX64InterruptDestinationModeLogical) const -> std::int32_t;

	private:
		std::vector<std::byte> m_InitState;
		Machine& m_Machine;
		Processor& m_Processor;
	};

}