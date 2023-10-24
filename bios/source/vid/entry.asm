
  extern _Main:near

_TEXT_HEDR segment use16 'HEDR'	
	org			0x0
	dw			0xaa55
	db			0x40
	jmp			_Main	
_TEXT_HEDR ends

	end