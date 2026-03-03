#ifndef _VELA_STDIO_H
#define _VELA_STDIO_H

#include <stddef.h>
#include <stdarg.h>

typedef void *FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define EOF      (-1)
#define BUFSIZ   1024
#define FILENAME_MAX 256

int sscanf(const char *str, const char *format, ...);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
int vfprintf(FILE *stream, const char *fmt, va_list ap);

#ifndef remove
#define remove(path) unlink(path)
#endif

#ifndef putchar
#define putchar(c) printf("%c", (c))
#endif

#define fflush(f) (0)
#define setbuf(f, b)
#define setvbuf(f, b, m, s) (0)
#define ferror(f) (0)
#define feof(f)   (0)
#define clearerr(f)
#define fileno(f) (-1)
#define tmpfile()  ((FILE*)0)
#define perror(s)  syslog(1, "[perror] %s", (s))

#endif
