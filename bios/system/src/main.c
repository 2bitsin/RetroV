void __attribute__((noreturn)) start () 
{
  unsigned short __far * _b800 = (unsigned short __far*)0xb8000000ul;
  for(unsigned i = 0; i < 80*25; i++) {
    _b800[i] = 0x0f00 | 'A';
  }
  while(1);
}