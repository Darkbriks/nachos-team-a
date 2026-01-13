#ifndef NOS_THREADS_H
#define NOS_THREADS_H

typedef unsigned int tid_t;

typedef struct {
    int entry;
    int arg;
    int user_sp;
    int tls_base;
} user_thread_args;

#endif
