#ifndef NOS_THREADS_H
#define NOS_THREADS_H

typedef unsigned int tid_t;

typedef struct {
    unsigned int entry;
    unsigned int user_sp;
    unsigned int tls_base;
    unsigned int flags;
    unsigned int clear_tid;
} user_thread_args;

// Thread flags

// First bit: detached / joinable
#define USER_THREAD_FLAG_DETACHED  (1 << 0)
#define USER_THREAD_FLAG_JOINABLE  (0 << 0)

#endif