#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <bios/vmcall.h>
#include <com/string.h>

static char const world_s[] = "World";

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  int i = 0;
  char buff[0x20];
  memset(buff, 0, 0x20);
	rev_sprnf(buff, "Hello %s Nr. %d!\n", world_s, i+=1u);
	write_log_string(buff);
	rev_sprnf(buff, "Hello %s Nr. %d!\n", world_s, i+=1u);
	write_log_string(buff);

  __asm { sti }
  for(;;) {
    __asm { hlt }
  }  
}
