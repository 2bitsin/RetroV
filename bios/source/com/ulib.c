#include <com/ulib.h>
#include <com/data.h>
#include <bios/vmcall.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

char __far* __watcall vsprnf(char __far* buffer, char const __far* format, va_list args_va)
{	
	char const __far* next_src;
	char __far* next_dst = buffer;
	union {
		int ival;
		unsigned int uval;
		long lval;
		unsigned long ulval;
		char const* sval;
		char cval;
	} u;	
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
		case 'x':
			u.uval = va_arg(args_va, unsigned int);
			utoa(u.uval, next_dst, 16);
			next_dst += strlen(next_dst);
			break;
		case 'l':
			switch (*(next_src + 1))
			{
			case 'u':
				u.ulval = va_arg(args_va, unsigned long);
				ultoa(u.ulval, next_dst, 10);
				next_dst += strlen(next_dst);
				next_src += 1u;
				break;
			case 'x':
				u.ulval = va_arg(args_va, unsigned long);
				ultoa(u.ulval, next_dst, 16);
				next_dst += strlen(next_dst);
				next_src += 1u;
				break;
			default:
				u.lval = va_arg(args_va, long);
				ltoa(u.lval, next_dst, 10);
				next_dst += strlen(next_dst);
				break;
			}
			break;
		case 's':
			u.sval = va_arg(args_va, char const*);
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
	return buffer;
}

char __far* __cdecl sprnf(char __far* buffer, char const __far* format, ...)
{
	va_list args_va;
	va_start(args_va, format);
	buffer = vsprnf(buffer, format, args_va);
	va_end(args_va);
	return buffer;
}

void __cdecl prnf(char const __far* format, ...)
{
	va_list args_va;
	va_start(args_va, format);
	vsprnf(&ebda.prnf_buf[0], format, args_va);
	write_log_string(&ebda.prnf_buf[0]);
	va_end(args_va);
}



