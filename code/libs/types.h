#ifndef NACHOS_LIB_TYPES_H
#define NACHOS_LIB_TYPES_H

/* ============================================================
 * Types definitions
 * ============================================================
 */

typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
typedef unsigned long long uint64_t;
typedef signed long long   int64_t;

typedef uint32_t size_t;
typedef int32_t  ssize_t;
typedef int32_t  ptrdiff_t;

typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;

/* ============================================================
 * General purpose constants
 * ============================================================
 */

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef PAGE_SIZE 
#define PAGE_SIZE 128 
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* ============================================================
 * Utility macros
 * ============================================================
 */

#define PACKED       __attribute__((packed))
#define ALIGNED(x)   __attribute__((aligned(x)))
#define NORETURN     __attribute__((noreturn))
#define UNUSED       __attribute__((unused))
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)

/* ============================================================
 * Thread macros
 * ============================================================
 */

#define JOINABLE 0
#define DETACHED 1

/* ============================================================
 * Semaphore macros
 * ============================================================
 */

typedef int sem_t;

#endif /* NACHOS_LIB_TYPES_H */
