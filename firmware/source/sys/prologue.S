.code16
.section .text.prologue
.global prologue
.global c_main
.global _ebda_seg

prologue:    
    jmp         c_main

