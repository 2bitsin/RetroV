#pragma  once

#include <stdint.h>
#include <stddef.h>

void __hlt(void);
#pragma aux __hlt = "hlt";

void __nop(void);
#pragma aux __nop = "nop";

void __cli(void);
#pragma aux __cli = "cli";

void __sti(void);
#pragma aux __sti = "sti";

void __outb(uint16_t port, uint8_t data);
#pragma aux __outb __parm [dx] [al] = "out dx, al";

void __outw(uint16_t port, uint16_t data);
#pragma aux __outw __parm [dx] [ax] = "out dx, ax";

void __rep_outsb(uint16_t port, uint8_t const _far* data, uint16_t size);
#pragma aux __rep_outsb __parm [dx] [ds si] [cx] = "rep outsb";

void __rep_outsw(uint16_t port, uint16_t const _far* data, uint16_t size);
#pragma aux __rep_outsw __parm [dx] [ds si] [cx] = "rep outsw";

uint8_t __inb(uint16_t port);
#pragma aux __inb __parm [dx] = "in al, dx" __value [al];

uint16_t __inw(uint16_t port);
#pragma aux __inw __parm [dx] = "in ax, dx" __value [ax];


void set_debug_flag(void);
#pragma aux set_debug_flag = \
	"pushf"					\
	"pop ax"				\
	"or ax, 0x100"	\
	"push ax"				\
	"popf"					;
