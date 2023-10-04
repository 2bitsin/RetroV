.code16
.global start
.global reset_vec
// 16bit code
.section .text.reset_vec 
reset_vec:
  jmp start