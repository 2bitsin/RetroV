#pragma once

#define PRNF_BUF_SIZE 128

typedef struct {
	char prnf_buf[PRNF_BUF_SIZE];
} ebda_type ;
                      
extern ebda_type ebda;

