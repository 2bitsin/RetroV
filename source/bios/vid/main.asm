
Header:
    db      0x55, 0xaa
    db      0x00
    jmp     short Prologue 

include     'prologue.asi'
include     '../com/debug.asi'
include     'fontbins.asi'
include     'strings.asi'
include     'epilogue.asi'