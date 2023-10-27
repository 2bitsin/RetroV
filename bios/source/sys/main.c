#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <bios/vmcall.h>
#include <com/ulib.h>
#include <com/intrin.h>

void video_init(uint8_t idx) 
{
  __asm
  {
    xor ah, ah
    mov al, idx
    int 0x10
  }
}

__declspec(noreturn) 
void __cdecl __loadds Main() 
{
  xrom_init();
  video_init(0x3);
  __sti();
  while(true) __hlt();  
}
