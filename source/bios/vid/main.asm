
Header:
    dw      0xaa55
    db      0x40
    jmp     Prologue 

include '../gen/hypercall.asi'
include 'prologue.asi'
include 'vidmode.asi'
include '../com/debug.asi'
include 'fontbins.asi'
include 'strings.asi'
include 'epilogue.asi'