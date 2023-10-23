#pragma once

void write_log_string(char const _far* str);

#pragma aux write_log_string \
  __parm [es di] = \
  "push word ptr 0xFF02" \
  "db 0x0f, 0x01, 0xd9" \
  "add sp, 2";



