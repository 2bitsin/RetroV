
_DATA segment use16 'DATA'
_DATA ends

_TEXT segment use16 'CODE'
_TEXT ends

_STACK segment use16 'STACK' AT 0x30
_STACK ends

_BSS segment use16 'BSS' AT 0x9FC0
_BSS ends

_BDA segment use16 'BDA' AT 0x0040
_BDA ends

  end
  