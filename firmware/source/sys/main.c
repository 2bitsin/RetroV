#include "com/inlasm.h"
#include "com/vmcall.h"

__far void __attribute__((interrupt)) Int0x10() {
  Debugger_WriteLogString("Hello, Interrupt!\n"); 
}


void CMain () 
{
  Debugger_WriteLogString("Hello, World!\n");
  for(;;) 
  {
    inlasm_hlt();
  }
}