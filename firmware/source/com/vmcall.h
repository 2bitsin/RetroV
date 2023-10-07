#pragma once

#include <stdint.h>
#include <stddef.h>

#include "com/macros.h"
#include "com/types.h"

#define svm_call(X) asm volatile("pushw %0\n" ".byte 0x0F,0x01,0xD9\n" "addw $2, %%sp\n" : : "i"(X));
#define vtx_call(X) asm volatile("pushw %0\n" ".byte 0x0F,0x01,0xC1\n" "addw $2, %%sp\n" : : "i"(X));

#if defined(USE_VTX_CALL) && USE_VTX_CALL == 1
  #define vm_call(X) vtx_call(X)
#else
  #define vm_call(X) svm_call(X)
#endif

void Debugger_ToggleUnrealMode(bool enable_v);
void Debugger_WriteLogString(char const* string_v);
void Debugger_WriteLogChar(char value_v);

