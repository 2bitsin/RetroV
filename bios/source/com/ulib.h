#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

char __far* vsprnf (char __far* buffer, char const __far* format, va_list args_va);
char __far* sprnf  (char  __far* buffer, char const __far* format, ...);
void	      prnf   (char const __far* format, ...);
