#include <stdint.h>
#include <stddef.h>

#include "com/vmcall.h"
#include "com/inlasm.h"
#include "com/memmap.h"

__far void __attribute__((interrupt))
Int10h_handler(void) 
{
	Debugger_WriteLogString("INT 0x10\n");
}

void c_main() 
{	
	Debugger_WriteLogString("RetroV BIOS version 0.1g.\n");
	IVT[0x10] = Int10h_handler;
	asm("int $0x10");
	for (;;) {
		inlasm_hlt();
	}
}