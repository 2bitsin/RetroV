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
		WHvX64RegisterCr0,
		WHvX64RegisterCr2,
		WHvX64RegisterCr3,
		WHvX64RegisterCr4,
		WHvX64RegisterCr8,
		WHvX64RegisterDr0,
		WHvX64RegisterDr1,
		WHvX64RegisterDr2,
		WHvX64RegisterDr3,
		WHvX64RegisterDr6,
		WHvX64RegisterDr7,
		WHvX64RegisterXCr0,
		WHvX64RegisterVirtualCr0,
		WHvX64RegisterVirtualCr3,
		WHvX64RegisterVirtualCr4,
		WHvX64RegisterVirtualCr8,
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
		WHvX64RegisterFpMmx0,
		WHvX64RegisterFpMmx1,
		WHvX64RegisterFpMmx2,
		WHvX64RegisterFpMmx3,
		WHvX64RegisterFpMmx4,
		WHvX64RegisterFpMmx5,
		WHvX64RegisterFpMmx6,
		WHvX64RegisterFpMmx7,
		WHvX64RegisterFpControlStatus,
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
	} 

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
	} 

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
	}
	
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
	};

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
	};

	//MAKE_REGISTER(L, l);
	MAKE_REGISTER(G, g);
	MAKE_REGISTER(I, i);
	
#undef MAKE_REGISTER

	struct Tr: public regiser_base<WHvX64RegisterTr>{ uint16_t tr; };
//	struct Ldtr: public regiser_base<WHvX64RegisterTr> { uint16_t ldtr; };

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