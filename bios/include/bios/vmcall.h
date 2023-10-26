#pragma once

#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
// 
//	Debugger device hypercalls
// 
//////////////////////////////////////////////////////////////////////////

#define HYPERCALL_DEBUG                     0xff00

#define HYPERCALL_DEBUG_TOGGLE_UNREAL_MODE  0xff00 
#ifdef __WATCOMC__
  void enable_unreal_mode(unsigned bool_v);
  #pragma aux enable_unreal_mode \
    __parm [bx] = \
    "push word ptr 0xff00" \
    "db 0x0f, 0x01, 0xd9" \
    "add sp, 2";
#endif


#define HYPERCALL_DEBUG_WRITE_LOG_CHAR      0xff01 
#ifdef __WATCOMC__
  void write_log_char(char chr_v);
  #pragma aux write_log_char		\
    __parm [bl] =								\
    "push word ptr 0xff01"			\
    "db 0x0f, 0x01, 0xd9"				\
    "add sp, 2"									;
#endif

#define HYPERCALL_DEBUG_WRITE_LOG_STRING    0xff02 
#ifdef __WATCOMC__
  void write_log_string(char const _far* str_v);
  #pragma aux write_log_string	\    
    __parm [ds si] =						\
    "xor cx, cx"								\
    "push word ptr 0xff02"			\
    "db 0x0f, 0x01, 0xd9"				\
    "add sp, 2"									\
    __modify [cx]								;
#endif

#define HYPERCALL_DEBUG_DEBUGGER_BREAK      0xffff 
#ifdef __WATCOMC__
  void debugger_break();
  #pragma aux debugger_break =	\    
    "push word ptr 0xffff"			\
    "db 0x0f, 0x01, 0xd9"				\
    "add sp, 2"									;
#endif

//////////////////////////////////////////////////////////////////////////
// 
//	Video device hypercalls
// 
//////////////////////////////////////////////////////////////////////////

#define HYPERCALL_VIDEO															0x0000

#define HYPERCALL_VIDEO_SET_MODE										0x0000

#define HYPERCALL_VIDEO_MODE_CHARACTER_COLOR_ATTR		0x0000
#define HYPERCALL_VIDEO_MODE_CHARACTER_MONO_ATTR		0x0001
#define HYPERCALL_VIDEO_MODE_GRAPHICAL_COLOR_IDX8		0x0002

#ifdef __WATCOMC__
		int32_t video_set_mode(uint16_t horiz_v, uint16_t vert_v, uint16_t mode_v, uint16_t flags_v);
		#pragma aux video_set_mode  	\
			__parm [bx][cx][dx][ax] =		\
			"push word ptr 0x0000"			\
			"db 0x0f, 0x01, 0xd9"				\
			"add sp, 2"									\
			__value [dx ax]							;				
#endif

