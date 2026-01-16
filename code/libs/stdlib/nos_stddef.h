#ifndef NOS_STDDEF_H
#define NOS_STDDEF_H

#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
typedef int ptrdiff_t; // Assuming 32-bit architecture (NachOS user programs are compiled in MIPS32)
#endif

#ifndef _MAX_ALIGN_T_DEFINED
#define _MAX_ALIGN_T_DEFINED
typedef union {
    long long ll;
    double d;
    long double ld;
    void* p;
} max_align_t __attribute__((aligned(__alignof__(long double))));
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef _OFFSET_OF_DEFINED
#define _OFFSET_OF_DEFINED
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#endif

#endif
