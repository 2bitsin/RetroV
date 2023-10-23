_DATA segment use16 'DATA'
_DATA ends

_TEXT segment use16 'CODE'
  extern _Main:near
_TEXT ends

_STACK segment use16 'STACK' AT 0x30
_STACK ends


_TEXT_HEDR segment use16 'HEDR'	
	org			0x0
	dw			0xaa55
	db			0x40
	jmp			_Main	
_TEXT_HEDR ends

	end