#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <bios/vmcall.h>
#include <com/ulib.h>


void set_debug_flag(void);

#pragma aux set_debug_flag = \
	"pushf"					\
	"pop ax"				\
	"or ax, 0x100"	\
	"push ax"				\
	"popf"					;

	


static char const world_s[] = "World";

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
	int i = 0;
    
	//set_debug_flag();

	prnf("Hello %s Nr. %d!\n", world_s, i+=1u);
	prnf("Hello %s Nr. %d!\n", world_s, i+=1u);

  __asm { sti }
  for(;;) {
    __asm { hlt }
  }  
}
