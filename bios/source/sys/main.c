#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <bios/vmcall.h>

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  int i = 0;
  char buff[0x20];
	//sprintf(buff, "Hello %i-th World!", 1337);	

  write_log_string(buff);



  __asm { sti }
  for(;;) {
    __asm { hlt }
  }  
}
