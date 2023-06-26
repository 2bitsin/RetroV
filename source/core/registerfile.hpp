#pragma once

#include <cstdint>
#include <cstddef>

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
		uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	#define DECLARE_REGISTER(N) union { uint64_t r##N; uint32_t e##N; uint16_t N; };
		DECLARE_REGISTER(si)
		DECLARE_REGISTER(di)
		DECLARE_REGISTER(bp)
		DECLARE_REGISTER(sp)
		DECLARE_REGISTER(ip)
	};

}