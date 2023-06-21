#pragma once

#include <cstdint>
#include <cstddef>


struct x64_registers
{
#define MAKE_REGISTER(name) \
	union { uint64_t r##name##x; uint32_t e##name##x; uint16_t name##x; struct { uint8_t al; uint8_t ah; } };
	MAKE_REGISTER(a)
	MAKE_REGISTER(b)
	MAKE_REGISTER(c)
	MAKE_REGISTER(d)
#undef MAKE_REGISTER

#define MAKE_REGISTER(name) \
	union { uint64_t r##name; uint32_t e##name; uint16_t name; };
	MAKE_REGISTER(si)
	MAKE_REGISTER(di)
	MAKE_REGISTER(bp)
	MAKE_REGISTER(sp)
	MAKE_REGISTER(ip)
#undef MAKE_REGISTER

	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	union {
		uint64_t rflags; 
		uint32_t eflags; 
		uint16_t flags; 
		uint8_t bflags;
	};

	struct _Descriptor
	{
		uint64_t base;
		uint32_t size;
		uint16_t value;
		uint16_t attr;
	} cs, ds, es, fs, gs, ss, ldtr;

	struct _Table
	{
		uint64_t base;
		uint16_t limit;
	} gdtr, idtr;

	uint16_t tr; 
	
	uint64_t cr0, cr2, cr3, cr4, cr8;
	uint64_t dr0, dr1, dr2, dr3, dr6, dr7;

};