#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core::detail
{
	static inline constexpr WHV_REGISTER_NAME const G_RegisterNames[] =
	{
		WHvX64RegisterRax,  // OK
 		WHvX64RegisterRcx,	// OK
		WHvX64RegisterRdx,	// OK
		WHvX64RegisterRbx,	// OK
		WHvX64RegisterRsp,	// OK
		WHvX64RegisterRbp,	// OK
		WHvX64RegisterRsi,	// OK
		WHvX64RegisterRdi,	// OK
		WHvX64RegisterR8,		// OK
		WHvX64RegisterR9,		// OK
		WHvX64RegisterR10,	// OK
		WHvX64RegisterR11,	// OK
		WHvX64RegisterR12,	// OK
		WHvX64RegisterR13,	// OK
		WHvX64RegisterR14,	// OK
		WHvX64RegisterR15,	// OK
		WHvX64RegisterRip,	// OK
		WHvX64RegisterRflags, // OK
		WHvX64RegisterEs,   // OK
		WHvX64RegisterCs,   // OK
		WHvX64RegisterSs,   // OK
		WHvX64RegisterDs,   // OK
		WHvX64RegisterFs,   // OK
		WHvX64RegisterGs,   // OK
		WHvX64RegisterLdtr, // OK ?
		WHvX64RegisterTr,   // OK ?
		WHvX64RegisterIdtr, // OK
		WHvX64RegisterGdtr,	// OK
		WHvX64RegisterCr0,  // OK
		WHvX64RegisterCr2,	// OK
		WHvX64RegisterCr3,	// OK
		WHvX64RegisterCr4,	// OK
		WHvX64RegisterCr8,	// OK
		WHvX64RegisterDr0,	// OK
		WHvX64RegisterDr1,	// OK
		WHvX64RegisterDr2,	// OK
		WHvX64RegisterDr3,	// OK
		WHvX64RegisterDr6,	// OK
		WHvX64RegisterDr7,	// OK
		WHvX64RegisterXCr0,	// OK
		WHvX64RegisterVirtualCr0, // OK
		WHvX64RegisterVirtualCr3,	// OK
		WHvX64RegisterVirtualCr4,	// OK
		WHvX64RegisterVirtualCr8,	// OK
		WHvX64RegisterFpMmx0, // OK
		WHvX64RegisterFpMmx1,	// OK
		WHvX64RegisterFpMmx2,	// OK
		WHvX64RegisterFpMmx3,	// OK
		WHvX64RegisterFpMmx4,	// OK
		WHvX64RegisterFpMmx5,	// OK
		WHvX64RegisterFpMmx6,	// OK
		WHvX64RegisterFpMmx7,	// OK
		WHvX64RegisterFpControlStatus,
		WHvX64RegisterXmm0,
		WHvX64RegisterXmm1,
		WHvX64RegisterXmm2,
		WHvX64RegisterXmm3,
		WHvX64RegisterXmm4,
		WHvX64RegisterXmm5,
		WHvX64RegisterXmm6,
		WHvX64RegisterXmm7,
		WHvX64RegisterXmm8,
		WHvX64RegisterXmm9,
		WHvX64RegisterXmm10,
		WHvX64RegisterXmm11,
		WHvX64RegisterXmm12,
		WHvX64RegisterXmm13,
		WHvX64RegisterXmm14,
		WHvX64RegisterXmm15,
		WHvX64RegisterXmmControlStatus,
	};
}

namespace core::regs
{
	struct register_component
	{};

	template<WHV_REGISTER_NAME Name>
	struct regiser_base: register_component {
		static inline constexpr auto const name = Name;
	};

#pragma pack(push, 1)

#define MAKE_REGISTER(L) \
	struct R##L##x: public regiser_base<WHvX64RegisterR##L##x> \
	{	\
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			uint64_t r##L##x; \
			uint32_t e##L##x; \
			uint16_t L##x; \
			struct { \
				uint8_t L##l; \
				uint8_t L##h;	\
			}; \
		}; \
	}; \
	static_assert(sizeof(R##L##x) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(a);
	MAKE_REGISTER(b);
	MAKE_REGISTER(c);
	MAKE_REGISTER(d);
#undef MAKE_REGISTER

#define MAKE_REGISTER(L) \
	struct R##L: public regiser_base<WHvX64RegisterR##L> \
	{	\
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			uint64_t r##L; \
			uint32_t e##L; \
			uint16_t L; \
			uint8_t L##l; \
		}; \
	}; \
	static_assert(sizeof(R##L) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(bp);
	MAKE_REGISTER(sp);
	MAKE_REGISTER(si);
	MAKE_REGISTER(di);
	MAKE_REGISTER(ip);
	MAKE_REGISTER(flags);

#undef MAKE_REGISTER

#define MAKE_REGISTER(L) \
  struct R##L: public regiser_base<WHvX64RegisterR##L> \
	{ \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			uint64_t r##L; \
			uint32_t r##L##d; \
			uint16_t r##L##w; \
			uint8_t  r##L##b; \
		}; \
	};	\
	static_assert(sizeof(R##L) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(8);
	MAKE_REGISTER(9);
	MAKE_REGISTER(10);
	MAKE_REGISTER(11);
	MAKE_REGISTER(12);
	MAKE_REGISTER(13);
	MAKE_REGISTER(14);
	MAKE_REGISTER(15);

#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l) \
	struct L: public regiser_base<WHvX64Register##L> { \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			struct \
			{ \
				uint64_t base; \
				uint32_t limit; \
				uint16_t selector; \
				union \
				{ \
					struct \
					{ \
						uint16_t segment_type : 4; \
						uint16_t non_system : 1; \
						uint16_t privilege : 2; \
						uint16_t present : 1; \
						uint16_t reserved : 4; \
						uint16_t available : 1; \
						uint16_t long_mode : 1; \
						uint16_t default_32bit : 1; \
						uint16_t granularity : 1; \
					}; \
					uint16_t attributes; \
				}; \
			} l; \
		}; \
	}; \
	static_assert(sizeof(L) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(Cs, cs);
	MAKE_REGISTER(Ds, ds);
	MAKE_REGISTER(Es, es);
	MAKE_REGISTER(Fs, fs);
	MAKE_REGISTER(Gs, gs);
	MAKE_REGISTER(Ss, ss);
	MAKE_REGISTER(Ldtr, ldtr); 

#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l) \
	struct L##dtr: public regiser_base<WHvX64Register##L##dtr> { \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			struct \
			{ \
				uint16_t pad[3]; \
				uint16_t limit; \
				uint64_t base; \
			} l##dtr; \
		}; \
	}; \
	static_assert(sizeof(L##dtr) == sizeof(WHV_REGISTER_VALUE))

	//MAKE_REGISTER(L, l);
	MAKE_REGISTER(G, g);
	MAKE_REGISTER(I, i);
	
#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l, y) \
	struct L: public regiser_base<WHvX64Register##L>{ \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			uint##y##_t l; \
		}; \
	}; \
	static_assert(sizeof(L) == sizeof(WHV_REGISTER_VALUE))
	MAKE_REGISTER(Tr, tr, 16);

#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l) \
	struct L: public regiser_base<WHvX64Register##L> { \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			union \
			{ \
				uint64_t q; \
				uint32_t d; \
				uint16_t w; \
				uint8_t  b; \
			} l; \
		}; \
	};

	MAKE_REGISTER(Cr0, cr0);
	MAKE_REGISTER(Cr2, cr2);
	MAKE_REGISTER(Cr3, cr3);
	MAKE_REGISTER(Cr4, cr4);
	MAKE_REGISTER(Cr8, cr8);
	MAKE_REGISTER(Dr0, dr0);
	MAKE_REGISTER(Dr1, dr1);
	MAKE_REGISTER(Dr2, dr2);
	MAKE_REGISTER(Dr3, dr3);
	MAKE_REGISTER(Dr6, dr6);
	MAKE_REGISTER(Dr7, dr7);
	MAKE_REGISTER(XCr0, xcr0);
	MAKE_REGISTER(VirtualCr0, vcr0);
	MAKE_REGISTER(VirtualCr3, vcr3);
	MAKE_REGISTER(VirtualCr4, vcr4);
	MAKE_REGISTER(VirtualCr8, vcr8);

#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l) \
	struct L: public regiser_base<WHvX64Register##L##l> { \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			uint64_t st##l; \
			uint64_t xmm##l; \
		}; \
	}; \
	static_assert(sizeof(L) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(FpMmx, 0);
	MAKE_REGISTER(FpMmx, 1);
	MAKE_REGISTER(FpMmx, 2);
	MAKE_REGISTER(FpMmx, 3);
	MAKE_REGISTER(FpMmx, 4);
	MAKE_REGISTER(FpMmx, 5);
	MAKE_REGISTER(FpMmx, 6);
	MAKE_REGISTER(FpMmx, 7);

#undef MAKE_REGISTER

	struct Fpu: public regiser_base<WHvX64RegisterFpControlStatus>
	{
		struct
		{
			uint16_t control;
			uint16_t status;
			uint8_t  tag;
			uint8_t  reserved1;
			uint16_t last_op;
			union
			{
				// Long Mode
				uint64_t last_rip;
				// 32 Bit Mode
				struct
				{
					uint32_t last_eip;
					uint16_t last_cs;
					uint16_t reserved2;
				};
			};
		} fpu;
		WHV_REGISTER_VALUE value;
	};

#pragma pack(pop)
}

namespace core
{

#pragma pack(push, 1)
	template <typename... Register_base>
	requires (std::derived_from<Register_base, regs::register_component> && ...)
	struct Registers: public std::type_identity<Register_base>::type... 
	{
		template <typename Processor>
		Registers(Processor const& vcpu_v) {
			WIN32_ERROR_ASSERT(Load(vcpu_v));
		}

		Registers() = default;

		template <typename Processor>
		auto load(Processor const& vcpu_v) -> std::int32_t {
			static constexpr WHV_REGISTER_NAME const s_names[] = { Register_base::name... };
			auto* const state_ptr = (WHV_REGISTER_VALUE*)std::addressof(*this);
			return vcpu_v.GetRegisters(s_names, { state_ptr, state_ptr + std::size(s_names) });
		}

		template <typename Processor>
		auto save(Processor const& vcpu_v) const -> std::int32_t {
			static constexpr WHV_REGISTER_NAME const s_names[] = { Register_base::name... };
			auto const* const state_ptr = (WHV_REGISTER_VALUE const*)std::addressof(*this);
			return vcpu_v.SetRegisters(s_names, { state_ptr, state_ptr + std::size(s_names) });
		}
	};	
#pragma pack(pop)
}