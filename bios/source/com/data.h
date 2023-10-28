#pragma once

#define PRNF_BUF_SIZE 128

#define _DATA __based(__segname("_DATA"))
#define _BSS  __based(__segname("_BSS" ))
#define _TEXT __based(__segname("_TEXT"))

#define _NULL __based(__segname("_NULL"))

typedef struct {
	char prnf_buf[PRNF_BUF_SIZE];
} ebda_type ;

extern ebda_type ebda;


