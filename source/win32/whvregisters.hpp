#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utils/algorithm.hpp>
#include <utils/metaprog.hpp>

#include <type_traits>
#include <algorithm>
#include <exception>
#include <cstdint>
#include <cstddef>
#include <tuple>
#include <span>

namespace win32::regs
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
//MAKE_REGISTER(flags);

#undef MAKE_REGISTER

	struct Rflags: 
		public register_base<WHvX64RegisterRflags> 
	{
		union
		{
			WHV_REGISTER_VALUE value;
			struct
			{
				uint64_t carry : 1;                     // 0 
				uint64_t always_one: 1;                 // 1
				uint64_t parity : 1;                    // 2
				uint64_t reserved_0 : 1;                // 3
				uint64_t adjust : 1;                    // 4
				uint64_t reserved_1 : 1;                // 5
				uint64_t zero : 1;                      // 6
				uint64_t sign : 1;                      // 7
				uint64_t trap : 1;                      // 8
				uint64_t interrupt : 1;                 // 9
				uint64_t direction : 1;                 // 10
				uint64_t overflow : 1;                  // 11
				uint64_t iopl : 2;                      // 12-13
				uint64_t nested_task : 1;               // 14
				uint64_t reserved_2 : 1;                // 15
				uint64_t resume : 1;                    // 16
				uint64_t v8086_mode : 1;                // 17
				uint64_t alignment_check : 1;           // 18
				uint64_t virtual_interrupt : 1;         // 19
				uint64_t virtual_interrupt_pending : 1; // 20
				uint64_t id : 1;                        // 21
				uint64_t reserved_3 : 42;               // 22-63
			} flag;
			uint8_t flagsl;
			uint16_t flags;
			uint32_t eflags;
			uint64_t rflags;
		};
	};

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

namespace win32
{

#pragma pack(push, 1)

	template <typename...T>
	struct WHvRegisters;

	template <typename... Register_base>	
	requires (std::derived_from<Register_base, regs::register_component> && ...)
	struct WHvRegisters<Register_base...>: public Register_base... 
	{
		WHvRegisters() = default;

		static inline auto Names() noexcept -> std::span<WHV_REGISTER_NAME const> {
			static constexpr WHV_REGISTER_NAME const s_names[] = { Register_base::name... };
			return { s_names, std::size(s_names) };
		};

		auto Values() noexcept -> std::span<WHV_REGISTER_VALUE> {
			return { (WHV_REGISTER_VALUE*)this, sizeof...(Register_base) };
		}

		auto Values() const noexcept -> std::span<WHV_REGISTER_VALUE const> {
			return { (WHV_REGISTER_VALUE*)this, sizeof...(Register_base) };
		}

	};	

	template <typename... R>
	requires (std::derived_from<R, regs::register_component> && ...)
	struct WHvRegisters<ump::type_list<R...>> : public WHvRegisters<R...> {};

	template <typename... List>
	requires (ump::concepts::type_list<List> && ...)
	struct WHvRegisters<List...>: public WHvRegisters<ump::concat_t<List...>> {};

	static_assert(std::is_trivial_v<WHvRegisters<regs::GeneralPurpose, regs::ControlAndDebug, regs::FloatingPoint>>	            
							&&std::is_trivially_constructible_v<WHvRegisters<regs::GeneralPurpose, regs::ControlAndDebug, regs::FloatingPoint>>
							&&std::is_trivially_copyable_v<WHvRegisters<regs::GeneralPurpose, regs::ControlAndDebug, regs::FloatingPoint>>
				   		&&std::is_trivially_destructible_v<WHvRegisters<regs::GeneralPurpose, regs::ControlAndDebug, regs::FloatingPoint>>);

	template <typename Processor, typename... T>
	struct WHvRegistersScoped: public WHvRegisters<T...>
	{
		
		WHvRegistersScoped(Processor& vcpu_v): m_Vcpu(&vcpu_v) {
			if (nullptr != m_Vcpu) {
				WIN32_ERROR_ASSERT(m_Vcpu->GetRegisters(*this));
			}
		}

		inline ~WHvRegistersScoped() noexcept(false) {
			if (nullptr != m_Vcpu) {
				auto status_v = m_Vcpu->SetRegisters(*this);
				if (ERROR_SUCCESS != status_v
					&& std::uncaught_exceptions()<1)
				{
					WIN32_ERROR_ASSERT(status_v);
				}
			} 
		}
	private:
		Processor* m_Vcpu;
	};

#pragma pack(pop)
}