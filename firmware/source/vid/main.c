#include <stdint.h>
#include <stddef.h>

void c_main() 
{
  uint16_t __far* q = (uint16_t __far*)0xb8000000ul;
  uint16_t i;
  for (i = 0; i < 80*25; i++) {
    q[i] = 0x0700 | 'Q';
  }
}