#include <stdint.h>
#include <stddef.h>

#include <vid/fonts.h>
#include <com/ulib.h>
#include <bios/vmcall.h>

__declspec(noreturn)
void __far __cdecl __loadds Main()
{
  write_log_string("RevBIOS VGA 0.1g...\n");
  install_int10h();
}