#include "pthread.h"
#include "tls.h"
#include "syscall.h"
#include "nos_threads.h"
#include "nos_string.h"
#include "types.h"

typedef struct pthread_attr_t {
    char flag; //DETACH_STATE SCOPE INHERIT_SCHEDULER SCHEDULING_POLICY
    unsigned short scheduling_priority;
    unsigned int guard_size;
    int stack_address;
    unsigned int stack_size;
} pthread_attr_t;

#define DETACH_STATE(value) (value<<0)
#define SCOPE(value) (value<<1)
#define INHERIT_SCHEDULER(value) (value<<2)
#define SCHEDULING_POLICY(value) (value<<3)

int pthread_attr_init(pthread_attr_t *attr){
    memset(attr, 0, sizeof(pthread_attr_t));
    attr->guard_size = PAGE_SIZE;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr){
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t * attr, int value){
    if (value != 0 && value != 1){
        attr->flag &= ~DETACH_STATE(0);
        attr->flag |= DETACH_STATE(value);
        return 0;
    }
    return 1;
}

int pthread_attr_setscope(pthread_attr_t * attr, int value){
    if (value != 0 && value != 1){
        attr->flag &= ~SCOPE(0);
        attr->flag |= SCOPE(value);
        return 0;
    }
    return 1;
}

int pthread_attr_setinheritscheduler(pthread_attr_t * attr, int value){
    if (value != 0 && value != 1){
        attr->flag &= ~INHERIT_SCHEDULER(0);
        attr->flag |= INHERIT_SCHEDULER(value);
        return 0;
    }
    return 1;
}

int pthread_attr_setschedulingpolicy(pthread_attr_t * attr, int value){
    if (value != 0 && value != 1){
        attr->flag &= ~SCHEDULING_POLICY(0);
        attr->flag |= SCHEDULING_POLICY(value);
        return 0;
    }
    return 1;
}

int pthread_attr_getdetachstate(pthread_attr_t * attr, int *value){
   *value = DETACH_STATE(attr->flag);
   return 0;
}

int pthread_attr_getscope(pthread_attr_t * attr, int *value){
   *value = SCOPE(attr->flag);
   return 0;
}

int pthread_attr_getinheritscheduler(pthread_attr_t * attr, int *value){
   *value = INHERIT_SCHEDULER(attr->flag);
   return 0;
}

int pthread_attr_getschedulingpolicy(pthread_attr_t * attr, int *value){
   *value = SCHEDULING_POLICY(attr->flag);
   return 0;
}

int pthread_attr_setscheduling_priority(pthread_attr_t * attr, size_t value){
    attr->scheduling_priority = value;
    return 0;
}

int pthread_attr_setguard_size(pthread_attr_t * attr, size_t value){
    attr->guard_size = value;
    return 0;
}

int pthread_attr_setstack_address(pthread_attr_t * attr, int value){
    attr->stack_address = value;
    return 0;
}

int pthread_attr_setstack_size(pthread_attr_t * attr, size_t value){
    attr->stack_size = value;
    return 0;
}

int pthread_attr_setstack(pthread_attr_t * attr, int addres, size_t size){
    attr->stack_size = size;
    attr->stack_address = addres;
    return 0;
}

int pthread_attr_getstack(pthread_attr_t * attr, int *addres, size_t *size){
    *size = attr->stack_size;
    *addres = attr->stack_address;
    return 0;
}

int pthread_attr_getscheduling_priority(pthread_attr_t * attr, size_t *value){
    *value = attr->scheduling_priority;
    return 0;
}

int pthread_attr_getguard_size(pthread_attr_t * attr, size_t *value){
    *value = attr->guard_size;
    return 0;
}

int pthread_attr_getstack_address(pthread_attr_t * attr, int *value){
    *value = attr->stack_address;
    return 0;
}

int pthread_attr_getstack_size(pthread_attr_t * attr, size_t *value){
    *value = attr->stack_size;
    return 0;
}


int pthread_create(tid_t *tid, pthread_attr_t * attr, typeof(void *(void *)) *fun, void * arg){

    tls_t tls = {
        &tls, 
        0, 
        ForkSelf(), 
        0, 
        attr ? (void *) attr->stack_address : NULL, 
        attr ? (unsigned int) attr->stack_size: 0, 
        attr ? attr->flag: 0,
        0,
        {NULL}
    };


    user_thread_args args = { 
        (unsigned int) fun, 
        arg, 
        attr ? attr->stack_address : 0,
        (unsigned int) &tls,
        attr ? attr->flag : 0,
        0
    };
    thread_create(&args);
    return 0;
}
