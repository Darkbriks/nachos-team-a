#ifndef PTHREAD_ATTR_H
#define PTHREAD_ATTR_H

#include "types.h"

typedef struct pthread_attr_t {
    char flag; //DETACH_STATE SCOPE INHERIT_SCHEDULER SCHEDULING_POLICY
    unsigned short scheduling_priority;
    unsigned int guard_size;
    int stack_address;
    unsigned int stack_size;
} pthread_attr_t;

#define JOINABLE 0
#define DETACHED 1

#define DETACH_STATE(value) (value<<0)
#define SCOPE(value) (value<<1)
#define INHERIT_SCHEDULER(value) (value<<2)
#define SCHEDULING_POLICY(value) (value<<3)

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);

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


#endif // PTHREAD_ATTR_H
