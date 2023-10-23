#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <com/vmcall.h>

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  char buff[0x10] = "Hello World!\n";
  write_log_string(buff);
  __asm { sti }
  for(;;) {
    __asm { hlt }
  }  
}
