#include "com/vmcall.h"
#include "com/hypercall.h"


__attribute__((regparmcall))
void Debugger_ToggleUnrealMode(char enable_v)
{

	asm("xorw %%bx, %%bx\n"
			"movb   %0, %%bl\n" : : "r" (enable_v));
	vm_call(HYPERCALL_DEBUG_TOGGLE_UNREAL_MODE);
}

__attribute__((regparmcall))
void Debugger_WriteLogString(char const * string_v) {
	asm("movw %0, %%si\n" 
			"movw $0, %%cx\n"
			: : "r" (string_v));	
	vm_call(HYPERCALL_DEBUG_WRITE_LOG_STRING);
}