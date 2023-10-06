.code16
.section .text.startup

.global startup
.global prologue
.global _stack_seg
.global _stack_ptr

startup:  
    movw      _stack_seg,   %sp
    movw      %sp,          %ss
    movw      _stack_ptr,   %sp
    jmp       prologue