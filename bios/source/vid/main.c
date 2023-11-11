#include <stdint.h>
#include <stddef.h>

#include <bios/vmcall.h>

#include <vid/int10h.h>
#include <vid/fonts.h>
#include <vid/vgaio.h>

#include <com/ulib.h>

__declspec(naked) 
__declspec(noreturn) 
void __far jmp_int0x10 (void) 
{ __asm { 
  int 0x6D
  iret  
}}

__declspec(noreturn)
void __far __cdecl __loadds Main()
{
  write_log_string("RevBIOS VGA build: " __DATE__ " " __TIME__ "\n");
  ivt_set(0x6D, &int0x10);
  ivt_set(0x10, &jmp_int0x10);
  set_video_mode(0x0d, 0);
  set_video_mode(0x0e, 0);
	set_video_mode(0x13, 0);
}