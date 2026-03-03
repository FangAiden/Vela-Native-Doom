#ifndef _VELA_ASSERT_H
#define _VELA_ASSERT_H

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) \
    ((expr) ? (void)0 : \
     (syslog(1, "[ASSERT] %s:%d: %s", __FILE__, __LINE__, #expr), abort()))
#endif

#endif
