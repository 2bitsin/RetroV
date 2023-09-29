#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utils/metaprog.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core::detail
{
	/*		
		WHvX64RegisterRax = 0x00000000,
		WHvX64RegisterRcx = 0x00000001,
		WHvX64RegisterRdx = 0x00000002,
		WHvX64RegisterRbx = 0x00000003,
		WHvX64RegisterRsp = 0x00000004,
		WHvX64RegisterRbp = 0x00000005,
		WHvX64RegisterRsi = 0x00000006,
		WHvX64RegisterRdi = 0x00000007,
		WHvX64RegisterR8 = 0x00000008,
		WHvX64RegisterR9 = 0x00000009,
		WHvX64RegisterR10 = 0x0000000A,
		WHvX64RegisterR11 = 0x0000000B,
		WHvX64RegisterR12 = 0x0000000C,
		WHvX64RegisterR13 = 0x0000000D,
		WHvX64RegisterR14 = 0x0000000E,
		WHvX64RegisterR15 = 0x0000000F,
		WHvX64RegisterRip = 0x00000010,
		WHvX64RegisterRflags = 0x00000011,
		WHvX64RegisterEs = 0x00000012,
		WHvX64RegisterCs = 0x00000013,
		WHvX64RegisterSs = 0x00000014,
		WHvX64RegisterDs = 0x00000015,
		WHvX64RegisterFs = 0x00000016,
		WHvX64RegisterGs = 0x00000017,
		WHvX64RegisterLdtr = 0x00000018,
		WHvX64RegisterTr = 0x00000019,
		WHvX64RegisterIdtr = 0x0000001A,
		WHvX64RegisterGdtr = 0x0000001B,
		WHvX64RegisterCr0 = 0x0000001C,
		WHvX64RegisterCr2 = 0x0000001D,
		WHvX64RegisterCr3 = 0x0000001E,
		WHvX64RegisterCr4 = 0x0000001F,
		WHvX64RegisterCr8 = 0x00000020,
		WHvX64RegisterDr0 = 0x00000021,
		WHvX64RegisterDr1 = 0x00000022,
		WHvX64RegisterDr2 = 0x00000023,
		WHvX64RegisterDr3 = 0x00000024,
		WHvX64RegisterDr6 = 0x00000025,
		WHvX64RegisterDr7 = 0x00000026,
		WHvX64RegisterXCr0 = 0x00000027,
		WHvX64RegisterVirtualCr0 = 0x00000028,
		WHvX64RegisterVirtualCr3 = 0x00000029,
		WHvX64RegisterVirtualCr4 = 0x0000002A,
		WHvX64RegisterVirtualCr8 = 0x0000002B,
		WHvX64RegisterXmm0 = 0x00001000,
		WHvX64RegisterXmm1 = 0x00001001,
		WHvX64RegisterXmm2 = 0x00001002,
		WHvX64RegisterXmm3 = 0x00001003,
		WHvX64RegisterXmm4 = 0x00001004,
		WHvX64RegisterXmm5 = 0x00001005,
		WHvX64RegisterXmm6 = 0x00001006,
		WHvX64RegisterXmm7 = 0x00001007,
		WHvX64RegisterXmm8 = 0x00001008,
		WHvX64RegisterXmm9 = 0x00001009,
		WHvX64RegisterXmm10 = 0x0000100A,
		WHvX64RegisterXmm11 = 0x0000100B,
		WHvX64RegisterXmm12 = 0x0000100C,
		WHvX64RegisterXmm13 = 0x0000100D,
		WHvX64RegisterXmm14 = 0x0000100E,
		WHvX64RegisterXmm15 = 0x0000100F,
		WHvX64RegisterFpMmx0 = 0x00001010,
		WHvX64RegisterFpMmx1 = 0x00001011,
		WHvX64RegisterFpMmx2 = 0x00001012,
		WHvX64RegisterFpMmx3 = 0x00001013,
		WHvX64RegisterFpMmx4 = 0x00001014,
		WHvX64RegisterFpMmx5 = 0x00001015,
		WHvX64RegisterFpMmx6 = 0x00001016,
		WHvX64RegisterFpMmx7 = 0x00001017,
		WHvX64RegisterFpControlStatus = 0x00001018,
		WHvX64RegisterXmmControlStatus = 0x00001019,
	*/
}

namespace core::regs
{
	struct register_component
	{};

	template<WHV_REGISTER_NAME Name>
	struct register_base: register_component {
		static inline constexpr auto const name = Name;
	};

#pragma pack(push, 1)

#define MAKE_REGISTER(L) \
	struct R##L##x: public register_base<WHvX64RegisterR##L##x> \
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
	struct R##L: public register_base<WHvX64RegisterR##L> \
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
  struct R##L: public register_base<WHvX64RegisterR##L> \
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
	struct L: public register_base<WHvX64Register##L> { \
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
	struct L##dtr: public register_base<WHvX64Register##L##dtr> { \
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
	struct L: public register_base<WHvX64Register##L>{ \
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
	struct L: public register_base<WHvX64Register##L> { \
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
	}; \
	static_assert(sizeof(L) == sizeof(WHV_REGISTER_VALUE))

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
	struct L##l: public register_base<WHvX64Register##L##l> { \
		union \
		{ \
			WHV_REGISTER_VALUE value; \
			struct \
			{ \
        uint64_t mantissa; \
				uint64_t exponent : 15; \
				uint64_t sign : 1; \
				uint64_t _: 48; \
			} st##l; \
	    union \
			{ \
				uint64_t q; \
			  uint32_t d[2]; \
				uint16_t w[4]; \
				uint8_t  b[8]; \
			  double   f8; \
				float    f4[2]; \
			} mmx##l; \
		}; \
	}; \
	static_assert(sizeof(L##l) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(FpMmx, 0);
	MAKE_REGISTER(FpMmx, 1);
	MAKE_REGISTER(FpMmx, 2);
	MAKE_REGISTER(FpMmx, 3);
	MAKE_REGISTER(FpMmx, 4);
	MAKE_REGISTER(FpMmx, 5);
	MAKE_REGISTER(FpMmx, 6);
	MAKE_REGISTER(FpMmx, 7);

#undef MAKE_REGISTER

#define MAKE_REGISTER(L, l) \
	struct L##l : public register_base<WHvX64Register##L##l>{ \
		union { \
			WHV_REGISTER_VALUE value; \
			union { \
				uint64_t q[2]; \
				uint32_t d[4]; \
				uint16_t w[8]; \
				uint8_t  b[16]; \
				double   f8[2]; \
				float    f4[4]; \
			} xmm##l; \
		}; \
	}; \
	static_assert(sizeof(L##l) == sizeof(WHV_REGISTER_VALUE))

	MAKE_REGISTER(Xmm, 0);
	MAKE_REGISTER(Xmm, 1);
	MAKE_REGISTER(Xmm, 2);
	MAKE_REGISTER(Xmm, 3);
	MAKE_REGISTER(Xmm, 4);
	MAKE_REGISTER(Xmm, 5);
	MAKE_REGISTER(Xmm, 6);
	MAKE_REGISTER(Xmm, 7);
	MAKE_REGISTER(Xmm, 8);
	MAKE_REGISTER(Xmm, 9);
	MAKE_REGISTER(Xmm, 10);
	MAKE_REGISTER(Xmm, 11);
	MAKE_REGISTER(Xmm, 12);
	MAKE_REGISTER(Xmm, 13);
	MAKE_REGISTER(Xmm, 14);
	MAKE_REGISTER(Xmm, 15);

#undef MAKE_REGISTER

	struct FpuControlStatus: public register_base<WHvX64RegisterFpControlStatus>
	{
		struct
		{
			uint16_t control;
			uint16_t status;
			uint8_t  tag;
			uint8_t  _1;
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
					uint16_t _2;
				};
			};
		} fpu;
		WHV_REGISTER_VALUE value;
	};

	struct XmmControlStatus : public register_base<WHvX64RegisterXmmControlStatus>
	{
		struct
		{
			union
			{
				// Long Mode
				uint64_t last_rdp;
				// 32 Bit Mode
				struct
				{
					uint32_t last_dp;
					uint16_t last_ds;
					uint16_t _1;
				};
			};
			uint32_t status_control;
			uint32_t status_control_mask;
		} xmm;
		WHV_REGISTER_VALUE value;
	};

	using GeneralPurpose = ump::type_list<Rax, Rcx, Rdx, Rbx, Rsp, Rbp, Rsi, Rdi, R8, R9, R10, R11, R12, R13, R14, R15, Rip, Rflags, Es, Cs, Ss, Ds, Fs, Gs>;
	using ControlAndDebug = ump::type_list<Ldtr, Tr, Idtr, Gdtr, Cr0, Cr2, Cr3, Cr4, Cr8, Dr0, Dr1, Dr2, Dr3, Dr6, Dr7, XCr0, VirtualCr0, VirtualCr3, VirtualCr4, VirtualCr8>;
	using FloatingPoint = ump::type_list<Xmm0, Xmm1, Xmm2, Xmm3, Xmm4, Xmm5, Xmm6, Xmm7, Xmm8, Xmm9, Xmm10, Xmm11, Xmm12, Xmm13, Xmm14, Xmm15, FpMmx0, FpMmx1, FpMmx2, FpMmx3, FpMmx4, FpMmx5, FpMmx6, FpMmx7, FpuControlStatus, XmmControlStatus>;

#pragma pack(pop)
}

namespace core
{

#pragma pack(push, 1)

	template <typename...T>
	struct Registers;

	template <typename... Register_base>	
	requires (std::derived_from<Register_base, regs::register_component> && ...)
	struct Registers<Register_base...>: public Register_base... 
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


	template <typename... R>
	requires (std::derived_from<R, regs::register_component> && ...)
	struct Registers<ump::type_list<R...>> : public Registers<R...> {};

	template <typename... List>
	requires (ump::concepts::type_list<List> && ...)
	struct Registers<List...>: public Registers<ump::concat_t<List...>> {};


#pragma pack(pop)
}