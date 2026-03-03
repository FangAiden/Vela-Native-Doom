#include "vela_doom.h"

int errno = 0;

static int _stdin_placeholder;
static int _stdout_placeholder;
static int _stderr_placeholder;
void *stdin  = &_stdin_placeholder;
void *stdout = &_stdout_placeholder;
void *stderr = &_stderr_placeholder;

int isalpha(int c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
int isprint(int c) { return c >= 0x20 && c <= 0x7E; }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }
int isxdigit(int c) { return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
int ispunct(int c) { return isprint(c) && !isalnum(c) && c != ' '; }
int iscntrl(int c) { return (c >= 0 && c < 0x20) || c == 0x7F; }
int isgraph(int c) { return isprint(c) && c != ' '; }
int toupper(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }
int tolower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

int abs(int x) { return x < 0 ? -x : x; }
long labs(long x) { return x < 0 ? -x : x; }
double fabs(double x) { return x < 0.0 ? -x : x; }
double ceil(double x) {
    int ix = (int)x;
    if (x > 0.0 && x != (double)ix) return (double)(ix + 1);
    return (double)ix;
}
double floor(double x) {
    int ix = (int)x;
    if (x < 0.0 && x != (double)ix) return (double)(ix - 1);
    return (double)ix;
}

static unsigned int _rand_seed = 1;

int rand(void) {
    _rand_seed = _rand_seed * 1103515245 + 12345;
    return (int)((_rand_seed >> 16) & 0x7FFFFFFF);
}

void srand(unsigned int seed) {
    _rand_seed = seed;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return (c == '\0') ? (char *)s : 0;
}

char *strrchr(const char *s, int c) {
    const char *last = 0;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == '\0') return (char *)s;
    return (char *)last;
}

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *d = (char *)malloc(len);
    if (d) memcpy(d, s, len);
    return d;
}

static char *_strtok_last = 0;

char *strtok(char *str, const char *delim) {
    if (str) _strtok_last = str;
    if (!_strtok_last) return 0;

    while (*_strtok_last) {
        const char *d = delim;
        int is_delim = 0;
        while (*d) {
            if (*_strtok_last == *d) { is_delim = 1; break; }
            d++;
        }
        if (!is_delim) break;
        _strtok_last++;
    }

    if (*_strtok_last == '\0') { _strtok_last = 0; return 0; }

    char *token = _strtok_last;

    while (*_strtok_last) {
        const char *d = delim;
        while (*d) {
            if (*_strtok_last == *d) {
                *_strtok_last = '\0';
                _strtok_last++;
                return token;
            }
            d++;
        }
        _strtok_last++;
    }

    _strtok_last = 0;
    return token;
}

char *strupr(char *s) {
    char *p = s;
    while (*p) { *p = toupper(*p); p++; }
    return s;
}

char *strlwr(char *s) {
    char *p = s;
    while (*p) { *p = tolower(*p); p++; }
    return s;
}

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int d = tolower(*s1) - tolower(*s2);
        if (d != 0) return d;
        s1++; s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && *s2) {
        int d = tolower(*s1) - tolower(*s2);
        if (d != 0) return d;
        s1++; s2++;
    }
    if (n == (size_t)-1) return 0;
    return tolower(*s1) - tolower(*s2);
}

char *strerror(int errnum) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "error %d", errnum);
    return buf;
}

int atexit(void (*func)(void)) {
    (void)func;
    return 0;
}

int access(const char *path, int mode) {
    (void)mode;
    char statbuf[128];
    memset(statbuf, 0, sizeof(statbuf));
    return stat(path, statbuf);
}

char *getcwd(char *buf, size_t size) {
    if (buf && size > 1) {
        buf[0] = '/';
        buf[1] = '\0';
    }
    return buf;
}

void *bsearch(const void *key, const void *base, size_t nmemb,
              size_t size, int (*compar)(const void *, const void *)) {
    size_t lo = 0, hi = nmemb;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const void *p = (const char *)base + mid * size;
        int cmp = compar(key, p);
        if (cmp < 0) hi = mid;
        else if (cmp > 0) lo = mid + 1;
        else return (void *)p;
    }
    return 0;
}

typedef __builtin_va_list va_list_t;
#define va_start_m(ap, last) __builtin_va_start(ap, last)
#define va_end_m(ap)         __builtin_va_end(ap)
#define va_arg_m(ap, type)   __builtin_va_arg(ap, type)

int sscanf(const char *str, const char *format, ...) {
    va_list_t ap;
    va_start_m(ap, format);
    int count = 0;
    const char *s = str;
    const char *f = format;

    while (*f && *s) {
        if (*f == '%') {
            f++;
            while (*f >= '0' && *f <= '9') f++;

            if (*f == 'd' || *f == 'i') {
                int *p = va_arg_m(ap, int *);
                int val = 0, neg = 0;
                while (isspace(*s)) s++;
                if (*s == '-') { neg = 1; s++; }
                else if (*s == '+') s++;
                if (!(*s >= '0' && *s <= '9')) break;
                while (*s >= '0' && *s <= '9') {
                    val = val * 10 + (*s - '0');
                    s++;
                }
                *p = neg ? -val : val;
                count++;
            } else if (*f == 'u') {
                unsigned int *p = va_arg_m(ap, unsigned int *);
                unsigned int val = 0;
                while (isspace(*s)) s++;
                if (!(*s >= '0' && *s <= '9')) break;
                while (*s >= '0' && *s <= '9') {
                    val = val * 10 + (*s - '0');
                    s++;
                }
                *p = val;
                count++;
            } else if (*f == 'x' || *f == 'X') {
                unsigned int *p = va_arg_m(ap, unsigned int *);
                unsigned int val = 0;
                while (isspace(*s)) s++;
                if (*s == '0' && (*(s+1) == 'x' || *(s+1) == 'X')) s += 2;
                while (1) {
                    if (*s >= '0' && *s <= '9') val = val * 16 + (*s - '0');
                    else if (*s >= 'a' && *s <= 'f') val = val * 16 + (*s - 'a' + 10);
                    else if (*s >= 'A' && *s <= 'F') val = val * 16 + (*s - 'A' + 10);
                    else break;
                    s++;
                }
                *p = val;
                count++;
            } else if (*f == 's') {
                char *p = va_arg_m(ap, char *);
                while (isspace(*s)) s++;
                while (*s && !isspace(*s)) *p++ = *s++;
                *p = '\0';
                count++;
            } else if (*f == 'c') {
                char *p = va_arg_m(ap, char *);
                *p = *s++;
                count++;
            } else if (*f == 'n') {
                int *p = va_arg_m(ap, int *);
                *p = (int)(s - str);
            }
            f++;
        } else if (isspace(*f)) {
            while (isspace(*s)) s++;
            f++;
        } else {
            if (*f != *s) break;
            f++; s++;
        }
    }

    va_end_m(ap);
    return count;
}

int vsnprintf(char *buf, size_t size, const char *fmt, __builtin_va_list ap) {
    if (!buf || size == 0) return 0;

    char *dst = buf;
    char *end = buf + size - 1;
    const char *f = fmt;

    while (*f && dst < end) {
        if (*f != '%') {
            *dst++ = *f++;
            continue;
        }
        f++;

        while (*f == '-' || *f == '+' || *f == ' ' || *f == '0' || *f == '#') f++;
        while (*f >= '0' && *f <= '9') f++;
        if (*f == '.') {
            f++;
            while (*f >= '0' && *f <= '9') f++;
        }
        int is_long = 0;
        if (*f == 'l') { is_long = 1; f++; }

        if (*f == 'd' || *f == 'i') {
            int val = is_long ? (int)__builtin_va_arg(ap, long) : __builtin_va_arg(ap, int);
            char tmp[16];
            int neg = 0, len = 0;
            if (val < 0) { neg = 1; val = -val; }
            if (val == 0) tmp[len++] = '0';
            else while (val > 0) { tmp[len++] = '0' + val % 10; val /= 10; }
            if (neg && dst < end) *dst++ = '-';
            int j;
            for (j = len - 1; j >= 0 && dst < end; j--) *dst++ = tmp[j];
        } else if (*f == 'u') {
            unsigned int val = is_long ? (unsigned int)__builtin_va_arg(ap, unsigned long)
                                       : __builtin_va_arg(ap, unsigned int);
            char tmp[16];
            int len = 0;
            if (val == 0) tmp[len++] = '0';
            else while (val > 0) { tmp[len++] = '0' + val % 10; val /= 10; }
            int j;
            for (j = len - 1; j >= 0 && dst < end; j--) *dst++ = tmp[j];
        } else if (*f == 'x' || *f == 'X') {
            unsigned int val = is_long ? (unsigned int)__builtin_va_arg(ap, unsigned long)
                                       : __builtin_va_arg(ap, unsigned int);
            char tmp[16];
            const char *hex = (*f == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";
            int len = 0;
            if (val == 0) tmp[len++] = '0';
            else while (val > 0) { tmp[len++] = hex[val & 0xF]; val >>= 4; }
            int j;
            for (j = len - 1; j >= 0 && dst < end; j--) *dst++ = tmp[j];
        } else if (*f == 's') {
            const char *s = __builtin_va_arg(ap, const char *);
            if (!s) s = "(null)";
            while (*s && dst < end) *dst++ = *s++;
        } else if (*f == 'c') {
            int c = __builtin_va_arg(ap, int);
            *dst++ = (char)c;
        } else if (*f == 'p') {
            unsigned int val = (unsigned int)__builtin_va_arg(ap, void *);
            if (dst + 2 < end) { *dst++ = '0'; *dst++ = 'x'; }
            char tmp[16];
            int len = 0;
            if (val == 0) tmp[len++] = '0';
            else while (val > 0) { tmp[len++] = "0123456789abcdef"[val & 0xF]; val >>= 4; }
            int j;
            for (j = len - 1; j >= 0 && dst < end; j--) *dst++ = tmp[j];
        } else if (*f == '%') {
            *dst++ = '%';
        } else {
            __builtin_va_arg(ap, int);
        }
        f++;
    }

    *dst = '\0';
    return (int)(dst - buf);
}

int vfprintf(void *stream, const char *fmt, __builtin_va_list ap) {
    char buf[1024];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n > 0) {
        syslog(6, "%s", buf);
    }
    return n;
}
