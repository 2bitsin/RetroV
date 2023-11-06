#pragma once

#include <stdint.h>
#include <stddef.h>

#define MF_LEGACY          0x0001
#define MF_GRAYSUM         0x0002
#define MF_NOPALETTE       0x0008
#define MF_CUSTOMCRTC      0x0800
#define MF_LINEARFB        0x4000
#define MF_NOCLEARMEM      0x8000
#define MF_VBEFLAGS        0xfe00

int __watcall __loadds set_video_mode(uint8_t index_v, int flags_v);

