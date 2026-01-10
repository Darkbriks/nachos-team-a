#ifndef TLS_H
#define TLS_H

#define TLS_MAX_KEYS 16 // Maximum thread-specific data keys

#define TLS_FLAG_DETACHED (1 << 0)
#define TLS_FLAG_JOINABLE (1 << 1)
#define TLS_FLAG_EXITED (1 << 2)

#ifdef IN_USER_MODE
typedef struct _tls {
    struct _tls* self;         // Self-pointer
    unsigned int tid;          // Thread ID
    unsigned int pid;          // Process ID
    int errno_val;             // Thread-local errno
    void* stack_base;          // Stack base address
    unsigned int stack_size;   // Stack size in bytes
    unsigned int flags;        // Thread flags
    void* retval;              // Return value from thread
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

static_assert(sizeof(void*) == 4, "Working with a kernel space in 64 bits and a user space in 32 bits is very funny, isn't it?");

#else

#define TLS_REGISTER           28

#define TLS_OFFSET_SELF        0
#define TLS_OFFSET_TID         4
#define TLS_OFFSET_PID         8
#define TLS_OFFSET_ERRNO       12 // WARNING: Any change here must be reflected in syscall_macro.S
#define TLS_OFFSET_STACK_BASE  16
#define TLS_OFFSET_STACK_SIZE  20
#define TLS_OFFSET_FLAGS       24
#define TLS_OFFSET_RETVAL      28
#define TLS_OFFSET_TSD         32

#define TLS_HEADER_SIZE        32
#define TLS_TOTAL_SIZE         (TLS_HEADER_SIZE + (TLS_MAX_KEYS * 4))

#endif

#endif