#define HYPERCALL_DEFINE_MAJOR(X, Y, Z) define X byte Y
#define HYPERCALL_DEFINE(X, Y, Z) define X word ((Y*0x100) + Z)
#define CONSTANT2_DEFINE(X, Y) define X word Y
#define CONSTANT4_DEFINE(X, Y) define X dword Y
#include "hypercall.h"