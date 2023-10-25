#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

char __far* __watcall vsprnf (char __far* buffer, char const __far* format, va_list args_va);
char __far* __cdecl   sprnf  (char  __far* buffer, char const __far* format, ...);
void	      __cdecl   prnf   (char const __far* format, ...);

inline void __far* __watcall make_fp(uint16_t seg, uint16_t off) 
{
  return (void __far*)(seg*0x10000ul + off);
}

#define MAKE_FP(T, seg, off) (T __far*)((seg)*0x10000ul + (off))
