Main:
  .Header:
    dw      0xaa55
    db      0x40
    jmp     Prologue 

  .Body:
    include '../com/hypercall.asi'
    include 'prologue.asi'
    include 'vidmode.asi'
    include 'apientry.asi'
    include '../com/debug.asi'
    include 'fontbins.asi'
    include 'strings.asi'
    include '../com/buildvars.asi'
    include 'epilogue.asi'