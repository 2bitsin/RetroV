#pragma once

#include <stdint.h>
#include <stddef.h>

#include "com/macros.h"

#pragma pack(push, 1)

typedef void __far * ivt_entry_t;
typedef uint16_t u16x4_t [4];

#define IVT ((ivt_entry_t __far *)0x0u)

#define BDA_com_io (*(u16x4_t __far *)(0x00400000u + 0x0000u))
#define BDA_lpt_io (*(u16x4_t __far *)(0x00400000u + 0x0008u))

#pragma pack(pop)