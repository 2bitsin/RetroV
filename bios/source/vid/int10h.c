#include <i86.h>

#include <vid/int10h.h>
#include <com/ulib.h>
#include <bios/vmcall.h>

typedef void __far* ivt_entry;

void __interrupt __far __loadds int0x10 (union INTPACK r) {
  write_log_string("Hello world from int 0x10!\n");
}

void install_int10h() 
{
  ivt_install(0x10, &int0x10);
}
