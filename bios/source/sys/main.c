#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <com/vmcall.h>

static const char _Hello [] = "Hello World!\n";

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  char buff[0x10];
  strcpy (buff, _Hello);
  write_log_string(buff);
  __asm { sti }
  for(;;) {
    __asm { hlt }
  }  
}
