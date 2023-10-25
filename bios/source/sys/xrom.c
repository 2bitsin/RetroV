#include <stdint.h>
#include <stddef.h>

#include <com/ulib.h>

#define XROM_SEARCH_BEG 0xC000
#define XROM_SEARCH_END 0xF000
#define XROM_SEARCH_INC 0x0080
#define XROM_MAGIC_SIGN 0xAA55
#define XROM_SECT_PARAS 0x0020

typedef void (__watcall __far* call_fun) (void);

#pragma pack(push, 1)
typedef struct xrom_s {
  uint16_t magic;
  uint8_t  sects;
  uint8_t  entry[1];
} xrom_type;
#pragma pack(pop)

void __watcall xrom_init(void) 
{  
  register uint16_t seg = XROM_SEARCH_BEG;
  register xrom_type __far* xrom_p=0;
  prnf("Searching for Extension ROMs...\n");
  while(seg < XROM_SEARCH_END) {
    xrom_p = make_fp(seg, 0);
    if (xrom_p->magic != XROM_MAGIC_SIGN) {
      seg += XROM_SEARCH_INC;
      continue;
    }
    prnf("Found Extension ROM at %x:0000.\n", seg);
    ((call_fun)&xrom_p->entry[0])();
    seg += XROM_SECT_PARAS*xrom_p->sects;
    continue;
  }
  prnf("Done initializing Extension ROMs.\n");
}