.code16
.section .text
.global prologue
.global c_main
.global _ebda_seg

prologue:    
    movw        _ebda_seg,  %ax
    movw        %ax,        %ds
    jmp         c_main

