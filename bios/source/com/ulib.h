#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

///////////////////////// 
//
//  LOGGING
//
/////////////////
char __far* __watcall vsprnf(char __far* buffer, char const __far* format, va_list args_va);
char __far* __cdecl sprnf(char  __far* buffer, char const __far* format, ...);
void __cdecl prnf(char const __far* format, ...);


///////////////////////// 
//
//  INTERRUPTS
//
/////////////////
typedef void __far* ivt_entry_t;

extern ivt_entry_t __far ivt[256];

inline void __watcall ivt_set(uint8_t index_v, ivt_entry_t handler_v) {	
	ivt[index_v] = handler_v;
}

inline ivt_entry_t __watcall ivt_get(uint8_t index_v) {	
	return ivt[index_v];
}


///////////////////////// 
//
//  GENERAL
//
/////////////////
inline void __far* __watcall make_fp(uint16_t seg, uint16_t off) {
  return (void __far*)(seg*0x10000ul + off);
}
#define MAKE_FP(T, seg, off) (T __far*)((seg)*0x10000ul + (off))
