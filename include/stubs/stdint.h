#ifndef _VELA_STDINT_H
#define _VELA_STDINT_H

#ifndef _VELA_TYPES_H
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef int                int32_t;
typedef unsigned long      uintptr_t;
#endif

typedef signed char        int8_t;
typedef short              int16_t;
typedef unsigned long long uint64_t;
typedef long long          int64_t;
typedef int                intptr_t;

#define UINT8_MAX   0xFF
#define UINT16_MAX  0xFFFF
#define UINT32_MAX  0xFFFFFFFFU
#define INT8_MAX    127
#define INT8_MIN    (-128)
#define INT16_MAX   32767
#define INT16_MIN   (-32768)
#define INT32_MAX   2147483647
#define INT32_MIN   (-2147483647 - 1)
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL
#define INT64_MAX   9223372036854775807LL
#define INT64_MIN   (-9223372036854775807LL - 1)
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#define SIZE_MAX    UINT32_MAX

#endif
