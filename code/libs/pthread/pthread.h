#ifndef PTHREAD_H
#define PTHREAD_H

#include "types.h"
#include "nos_threads.h"

#define MAX_THREAD 30
#define MAX_PROCESS 15
typedef struct pthread_attr_t pthread_attr_t;
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

int pthread_attr_setdetachstate(pthread_attr_t * attr, int value);
int pthread_attr_setscope(pthread_attr_t * attr, int value);
int pthread_attr_setinheritscheduler(pthread_attr_t * attr, int value);
int pthread_attr_setschedulingpolicy(pthread_attr_t * attr, int value);

int pthread_attr_getdetachstate(pthread_attr_t * attr, int *value);
int pthread_attr_getscope(pthread_attr_t * attr, int *value);
int pthread_attr_getinheritscheduler(pthread_attr_t * attr, int *value);
int pthread_attr_getschedulingpolicy(pthread_attr_t * attr, int *value);

int pthread_attr_setscheduling_priority(pthread_attr_t * attr, size_t value);
int pthread_attr_setguard_size(pthread_attr_t * attr, size_t value);
int pthread_attr_setstack_address(pthread_attr_t * attr, int value);
int pthread_attr_setstack_size(pthread_attr_t * attr, size_t value);

int pthread_attr_getstack_size(pthread_attr_t * attr, size_t *value);
int pthread_attr_getstack_address(pthread_attr_t * attr, int *value);
int pthread_attr_getguard_size(pthread_attr_t * attr, size_t *value);
int pthread_attr_getscheduling_priority(pthread_attr_t * attr, size_t *value);

int pthread_attr_setstack(pthread_attr_t * attr, int addres, size_t size);
int pthread_attr_getstack(pthread_attr_t * attr, int *addres, size_t *size);

int pthread_create(pthread_t *thread, pthread_attr_t * attr, typeof(void *(void *)) *fun, void * arg);
void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
void pthread_destroy(pthread_t thread);
pthread_t pthread_self();

#endif // PTHREAD_H
