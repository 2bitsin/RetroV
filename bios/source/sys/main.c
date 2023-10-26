#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <bios/vmcall.h>
#include <com/ulib.h>
#include <com/intrin.h>

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  xrom_init();
  write_log_string("Testing int 0x10!\n");
  __asm { int 0x10 };
  write_log_string("Done testing int 0x10!\n");
  __sti();
  while(true) __hlt();  
}
