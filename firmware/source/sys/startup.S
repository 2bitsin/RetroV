  .code16
  .section .startup
  .global CMain
  .global startup
startup:
  movw      $0x030,     %sp
  movw      %sp,        %ss
  movw      $0x100,     %sp
  pushw     %cs
  popw      %ds
  pushw     %cs
  popw      %es
  jmp       CMain
