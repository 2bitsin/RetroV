#include <i86.h>

#include <vid/int10h.h>
#include <com/ulib.h>
#include <bios/vmcall.h>

typedef void __far* ivt_entry;

void __interrupt __far __loadds int0x10 (union INTPACK r) {
  r.x.ax = 1337;
  write_log_string("Hello World from int 0x10!\n");
}

void install_int10h() 
{
  __segment ivt_seg = 0;
  ivt_entry __based(ivt_seg) * ivt_table = 0;
  ivt_table[0x10] = &int0x10;
}
