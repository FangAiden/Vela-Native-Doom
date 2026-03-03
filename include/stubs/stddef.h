#ifndef _VELA_STDDEF_H
#define _VELA_STDDEF_H

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int size_t;
typedef int          ptrdiff_t;

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
