#include <com/string.h>

#include <stdlib.h>
#include <string.h>

char* rev_sprnf(char* buffer, const char* format, ...)
{
	va_list args_va;
	const char* next_src;
	char* next_dst = buffer;
	union {
		int ival;
		unsigned int uval;
		long lval;
		unsigned long ulval;
		const char* sval;
		char cval;
	} u;
	va_start(args_va, format);
	for (next_src = format; *next_src; ++next_src)
	{
		if (*next_src != '%')
		{
			*next_dst++ = *next_src;
			continue;
		}
		switch (*++next_src)
		{
		case 'i':
		case 'd':
			u.ival = va_arg(args_va, int);
			itoa(u.ival, next_dst, 10);
			next_dst += strlen(next_dst);
			break;
		case 'u':
			u.uval = va_arg(args_va, unsigned int);
			utoa(u.uval, next_dst, 10);
			next_dst += strlen(next_dst);
			break;
		case 'l':
			if (*(next_src + 1) == 'u')
			{
				u.ulval = va_arg(args_va, unsigned long);
				ultoa(u.ulval, next_dst, 10);
				next_dst += strlen(next_dst);
				next_src += 1u;
				/* Skip the 'u' character */
			}
			else
			{
				u.lval = va_arg(args_va, long);
				ltoa(u.lval, next_dst, 10);
				next_dst += strlen(next_dst);
			}
			break;
		case 's':
			u.sval = va_arg(args_va, const char*);
			strcpy(next_dst, u.sval);
			next_dst += strlen(next_dst);
			break;
		case 'c':
			u.cval = (char)va_arg(args_va, int);
			*next_dst++ = u.cval;
			break;
		default:
			/* Unsupported format specifier */
			*next_dst++ = '%';
			*next_dst++ = *next_src;
			break;
		}
	}
	*next_dst = '\0';  /* Null-terminate the string */
	va_end(args_va);
	return buffer;
}
