#ifndef _VELA_STRING_H
#define _VELA_STRING_H

#include <stddef.h>

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strtok(char *str, const char *delim);
char *strdup(const char *s);
char *strupr(char *s);
char *strlwr(char *s);
int   strcasecmp(const char *s1, const char *s2);
int   strncasecmp(const char *s1, const char *s2, size_t n);
char *strerror(int errnum);

#endif
