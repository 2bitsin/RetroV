#include <vid/int10h.h>
#include <vid/mode.h>

#include <com/intrin.h>
#include <com/ulib.h>

#include <bios/vmcall.h>

void __interrupt __loadds __far int0x10(union INTPACK r)
{
  switch (r.h.ah)
  {
  case 0x00:
    set_video_mode(r.h.al);
    return;
  default:
    prnf("Invalid INT 0x10 call, AX=%x\n", r.x.ax);
  }
}