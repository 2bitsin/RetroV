#pragma  once

void __hlt(void);
#pragma aux __hlt = "hlt";

void __nop(void);
#pragma aux __nop = "nop";

void __cli(void);
#pragma aux __cli = "cli";

void __sti(void);
#pragma aux __sti = "sti";


void set_debug_flag(void);
#pragma aux set_debug_flag = \
	"pushf"					\
	"pop ax"				\
	"or ax, 0x100"	\
	"push ax"				\
	"popf"					;
