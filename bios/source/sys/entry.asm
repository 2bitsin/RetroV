
_DATA segment use16 'DATA'
_DATA ends

_TEXT segment use16 'CODE'
  extern _Main:near
_TEXT ends

_STACK segment use16 'STACK' AT 0x30
_STACK ends

_TEXT_FFFF0 segment use16 'FFF0'
  
    public FFFF0

  FFFF0:
    cli
    mov     sp,   0x30
    mov     ss,   sp
    mov     sp,   0x100
    push    cs
    pop     ds
    push    cs
    pop     es
    jmp     _Main

_TEXT_FFFF0 ends

  end