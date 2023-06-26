#pragma once

#include <cstdint>
#include <cstddef>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

namespace core
{

	struct RegisterFile
	{
	#define DECLARE_REGISTER(N) union { uint64_t r##N##x; uint32_t e##N##x; uint16_t N##x; struct{ uint8_t N##l, N##h; }; };
		DECLARE_REGISTER(a)
		DECLARE_REGISTER(b)
		DECLARE_REGISTER(c)
		DECLARE_REGISTER(d)
	#undef DECLARE_REGISTER
	#define DECLARE_REGISTER(N) union { uint64_t r##N; uint32_t e##N; uint16_t N; };
		DECLARE_REGISTER(si)
		DECLARE_REGISTER(di)
		DECLARE_REGISTER(bp)
		DECLARE_REGISTER(sp)
		uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
		DECLARE_REGISTER(ip)
		DECLARE_REGISTER(flags);
	#undef DECLARE_REGISTER
	#pragma pack(push, 1)
		struct { uint64_t cs_base; uint32_t cs_size; uint16_t cs; uint16_t cs_attr; };
		struct { uint64_t ds_base; uint32_t ds_size; uint16_t ds; uint16_t ds_attr; };
		struct { uint64_t es_base; uint32_t es_size; uint16_t es; uint16_t es_attr; };
		struct { uint64_t fs_base; uint32_t fs_size; uint16_t fs; uint16_t fs_attr; };
		struct { uint64_t gs_base; uint32_t gs_size; uint16_t gs; uint16_t gs_attr; };
		struct { uint64_t ss_base; uint32_t ss_size; uint16_t ss; uint16_t ss_attr; };
	#pragma pack(pop)

		static inline constexpr const WHV_REGISTER_NAME Layout[] =
		{
			/*  0 */ WHvX64RegisterRax,
			/*  1 */ WHvX64RegisterRbx,
			/*  2 */ WHvX64RegisterRcx,
			/*  3 */ WHvX64RegisterRdx,
			/*  4 */ WHvX64RegisterRsi,
			/*  5 */ WHvX64RegisterRdi,
			/*  6 */ WHvX64RegisterRbp,
			/*  7 */ WHvX64RegisterRsp,
			/*  8 */ WHvX64RegisterR8,
			/*  9 */ WHvX64RegisterR9,
			/* 10 */ WHvX64RegisterR10,
			/* 11 */ WHvX64RegisterR11,
			/* 12 */ WHvX64RegisterR12,
			/* 13 */ WHvX64RegisterR13,
			/* 14 */ WHvX64RegisterR14,
			/* 15 */ WHvX64RegisterR15,
			/* 16 */ WHvX64RegisterRip,
			/* 17 */ WHvX64RegisterRflags,
			/* 18 */ WHvX64RegisterCs,
			/* 19 */ WHvX64RegisterDs,
			/* 20 */ WHvX64RegisterEs,
			/* 21 */ WHvX64RegisterFs,
			/* 22 */ WHvX64RegisterGs,
			/* 23 */ WHvX64RegisterSs
		};

	};

}