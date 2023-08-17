		org 0x0000
		
		VMCallInstruction equ vmcall

		include 'variables.asi'
		include 'prologue.asi'
		include 'debug.asi'
		include 'intvectbl.asi'
		include 'msrs.asi'
		include 'synic.asi'
		include 'strings.asi'
		include 'epilogue.asi'