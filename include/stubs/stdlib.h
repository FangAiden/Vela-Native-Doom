#ifndef _VELA_STDLIB_H
#define _VELA_STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX     0x7FFFFFFF

int abs(int x);
long labs(long x);

int rand(void);
void srand(unsigned int seed);

int atexit(void (*func)(void));

void *bsearch(const void *key, const void *base, size_t nmemb,
              size_t size, int (*compar)(const void *, const void *));

#endif
