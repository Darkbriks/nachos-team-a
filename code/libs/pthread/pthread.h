#ifndef PTHREAD_H
#define PTHREAD_H

#include "types.h"
#include "nos_threads.h"
#include "pthread_attr.h"

#define MAX_THREAD 30
#define MAX_PROCESS 15

#define DEFAULT_STACK_SIZE (8 * 128)
#define INT32_MAX 2147483647

typedef int pthread_t;

typedef struct pthread{
    pthread_t tid;
    unsigned int pid;

    int exited;
    int detached;

    int futex;

    void *retval;

    void *stack_base;
    size_t stack_size;

    void *tls_base;
} pthread_lib;

int pthread_create(pthread_t *thread, pthread_attr_t * attr, typeof(void *(void *)) *fun, void * arg);
void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
void pthread_destroy(pthread_t thread);
pthread_t pthread_self();

#endif // PTHREAD_H
