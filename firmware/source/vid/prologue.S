.code16
.section .text.prologue

.global c_main
.global startup
.global _rom_len

startup:
  .short    0xaa55
  .byte     _rom_len
  jmp       c_main

