    org 0x0000
  
    VMCallInstruction equ vmmcall
  
    include 'variables.asi'
    include 'prologue.asi'
    include '../com/debug.asi'
    include 'bigloop.asi'
    include 'intvectbl.asi'
    include 'optionrom.asi'
	include 'msrs.asi'
    include 'lapic.asi'
	include 'synic.asi'
    include 'strings.asi'
    include 'epilogue.asi'
