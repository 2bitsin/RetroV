#include <stdint.h>
#include <stddef.h>


static const char string_s[] = "Hello World!\n";

int __cdecl __loadds Main() 
{
  uint16_t __far *  target_v = (uint16_t __far *)0xB8000000;
  uint16_t i=0, j=0;
  for (;i < 80*25; ++i, ++j) {
    if (string_s[j] == 0) j = 0;
    target_v[i] = 0x0700u + string_s[j];   
  }
  return 0;
}
