#include <stdint.h>
#include <stddef.h>

#include "com/vmcall.h"
#include "com/inlasm.h"
#include "com/memmap.h"

#include "sys/int10h.h"

void c_main() 
{	
	Debugger_WriteLogString("RetroV BIOS version 0.1g.\n" 
													__DATE__ " " __TIME__ "\n");
	IVT[0x10] = Int10h_handler;
	asm("int $0x10");
	Debugger_WriteLogString("What's up ?\n");
	for (;;) {
		inlasm_hlt();
	}
}