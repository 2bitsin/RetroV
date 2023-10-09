#include "int10h.h"

#include "com/vmcall.h"

__far void __attribute__((interrupt))
Int10h_handler(void)
{
	asm("push %cs\n" 
			"push %cs\n"
			"pop %ds\n"
			"pop %es\n");
	Debugger_WriteLogString("INT 0x10\n");
}
