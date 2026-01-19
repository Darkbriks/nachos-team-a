#ifndef TLS_H
#define TLS_H

#define TLS_MAX_KEYS 32 // Maximum thread-specific data keys

#ifdef IN_USER_MODE

typedef struct _tls {
    struct _tls* self;         // Self-pointer
    int errno_val;             // Thread-local errno
    int pthread_ptr;           // Pointer on user thread struct
    void* tsd[TLS_MAX_KEYS];   // Thread-specific data slots
} tls_t;

/**
 * @brief Retrieve the current thread's TLS structure.
 *
 * This function reads the global pointer register to obtain
 * the address of the TLS structure for the currently executing thread.
 *
 * This is more efficient than making a system call to get the TLS.
 *
 * @return Pointer to the current thread's TLS structure.
 */
static inline tls_t* __get_tls() {
    tls_t* tls;
    __asm__ __volatile__ (
        "move %0, $gp"
        : "=r" (tls)
    );
    return tls;
}

// static_assert(sizeof(void*) == 4, "Working with a kernel space in 64 bits and a user space in 32 bits is very funny, isn't it?");

#endif

#define TLS_REGISTER           28

#define TLS_OFFSET_SELF        0
#define TLS_OFFSET_ERRNO       4 // WARNING: Any change here must be reflected in syscall_macro.S
#define TLS_OFFSET_TSD         8
#define TLS_OFFSET_PTHREAD_PTR 12

#define TLS_HEADER_SIZE        16
#define TLS_TOTAL_SIZE         (TLS_HEADER_SIZE + (TLS_MAX_KEYS * 4))

#endif