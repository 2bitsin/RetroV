#pragma once

#include <stdint.h>
#include <stddef.h>

#define Q_always_inline __attribute__((always_inline))

#define Q_stringify(X) #X
#define Q_expand_stringfy(X) Q_stringify(X)
#define Q_regcall __attribute__((regparmcall))
