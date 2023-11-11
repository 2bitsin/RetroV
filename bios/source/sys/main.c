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
//video_init(0x0);
//video_init(0x1);
//video_init(0x2);
//video_init(0x3);
//video_init(0x4);
//video_init(0x5);
//video_init(0x6);
//video_init(0x7);
//video_init(0x0D);
//video_init(0x0E);
//video_init(0x0F);
//video_init(0x10);
//video_init(0x11);
//video_init(0x12);
//video_init(0x13);
  __sti();
  while(true) __hlt();  
}
