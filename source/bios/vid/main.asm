
Header:
    dw      0xaa55
    db      0x00
    jmp     Prologue 

include     'prologue.asi'
include     '../com/debug.asi'
include     'fontbins.asi'
include     'strings.asi'
include     'epilogue.asi'