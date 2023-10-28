#include <stdint.h>
#include <stddef.h>

#include <vid/int10h.h>
#include <vid/fonts.h>
#include <com/ulib.h>
#include <bios/vmcall.h>

__declspec(noreturn)
void __far __cdecl __loadds Main()
{
  write_log_string("RevBIOS VGA 0.1g...\n");
  ivt_set(0x10, &int0x10);
}