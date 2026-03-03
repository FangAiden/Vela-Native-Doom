#ifndef _VELA_STRINGS_H
#define _VELA_STRINGS_H

#include <string.h>

static inline void bzero(void *s, unsigned int n) { memset(s, 0, n); }
static inline void bcopy(const void *src, void *dst, unsigned int n) { memmove(dst, src, n); }

#endif
