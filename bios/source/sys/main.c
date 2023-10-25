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
  __sti();
  while(true) __hlt();  
}
