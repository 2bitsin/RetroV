#include <stdint.h>
#include <stddef.h>

#include "com/vmcall.h"
#include "com/inlasm.h"

static char const hello_s[] = "RetroV BIOS version 0.1g.\n";

void c_main() 
{	
	Debugger_WriteLogString(hello_s);
	for (;;) {
		inlasm_hlt();
	}
}