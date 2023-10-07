#include "com/vmcall.h"
#include "com/hypercall.h"

void Debugger_ToggleUnrealMode(bool enable_v)
{
	asm("xchgw %0, %%bx\n" 
			: 
			: "r" ((uint16_t)enable_v)
			: "bx");
	// BX[0] => Enable
	vm_call(HYPERCALL_DEBUG_TOGGLE_UNREAL_MODE);
}

void Debugger_WriteLogChar(char char_v) {
	asm("xchgw %0, %%bx\n" 
			: 
			: "r" ((uint16_t)char_v)
			: "bx");
	// BX[0:7] => Char
	vm_call(HYPERCALL_DEBUG_WRITE_LOG_CHAR);
}

void Debugger_WriteLogString(char const * string_v) {
	asm("xchg %%ax, %%si\n" 
			"xorw %%cx, %%cx\n"
			: 
			: "r" (string_v) 
			: "si", "cx");	
	// DS:SI => String
	// CX    => Length
	vm_call(HYPERCALL_DEBUG_WRITE_LOG_STRING);
}