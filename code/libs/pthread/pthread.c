#include "pthread.h"
#include "tls.h"
#include "syscall.h"
#include "nos_threads.h"
#include "nos_string.h"
#include "nos_mem.h"
#include "types.h"

int mem_alloc_init = 0;
int mem_init_amount = 10000; // TODO adjust initial memory size

#define INIT_MEMORY_ALLOCATOR()    \
    if (mem_alloc_init == 0) {      \
        mem_alloc_init = 1;         \
        mem_init(mem_init_amount); \
    }

#define DEFAULT_STACK_SIZE (8 * 128)
#define INT32_MAX 2147483647

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


#define GET_PTR_AND_CATCH_NULL_RETUN0(TID)    \
    pthread_lib* thread_ptr = get_thread_by_tid(TID);  \
    if (! thread_ptr){                                 \
        return 0;                                      \
    }
                                                        
#define GET_PTR_AND_CATCH_NULL(TID)    \
    pthread_lib* thread_ptr = get_thread_by_tid(TID);  \
    if (! thread_ptr){                                 \
        return;                                        \
    }
    

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

struct thread_start_args {
    void *(*start_routine)(void *);
    void *arg;
};

void thread_start_wrapper(void *arg) {
    struct thread_start_args* start_args = (struct thread_start_args*) arg;
    void *(*start_routine)(void *) = start_args->start_routine;
    void *routine_arg = start_args->arg;

    mem_free(start_args);

    void* retval = start_routine(routine_arg);

    pthread_exit(retval); // No longer need to manipulate asm registers to auto exit
}

pthread_lib* array_tid[MAX_PROCESS][MAX_THREAD];

static inline pthread_lib* __pthread_self(){
    return (pthread_lib*)__get_tls()->tsd[0]; // Assuming TSD index 0 stores pthread_t pointer
}

pthread_lib* get_thread_by_tid(pthread_t thread){
    return array_tid[ForkSelf()][thread];
}

int pthread_create(pthread_t *thread, pthread_attr_t * attr, typeof(void *(void *)) *fun, void * arg) {
    INIT_MEMORY_ALLOCATOR();

    pthread_lib* t = (pthread_lib*) mem_alloc(sizeof(pthread_lib));
    if (t == NULL) {
        return -1;
    }

    t->exited = 0;
    t->detached = attr ? (attr->flag & DETACH_STATE(1)) != 0 : 0;
    t->futex = 0;
    t->retval = NULL;
    t->stack_size = attr && attr->stack_size > 0 ? attr->stack_size : DEFAULT_STACK_SIZE;

    // Use mmap to allocate stack without guard page for now
    t->stack_base = (void *)mmap(NULL, t->stack_size);
    if (t->stack_base == NULL) {
        mem_free(t);
        return -1;
    }

    t->tls_base = mem_alloc(sizeof(tls_t));
    if (t->tls_base == NULL) {
        munmap(t->stack_base);
        mem_free(t);
        return -1;
    }

    // TODO: Initialize TLS structure
    tls_t* tls = (tls_t*)t->tls_base;
    tls->self = tls;
    tls->stack_base = t->stack_base;
    tls->stack_size = t->stack_size;
    tls->retval = NULL;
    tls->tsd[0] = (void*)t;

    struct thread_start_args* start_args = (struct thread_start_args*) mem_alloc(sizeof(struct thread_start_args));
    if (start_args == NULL) {
        mem_free(t->tls_base);
        munmap(t->stack_base);
        mem_free(t);
        return -1;
    }

    start_args->start_routine = fun;
    start_args->arg = arg;

    user_thread_args uargs;
    uargs.entry = (unsigned int)thread_start_wrapper;
    uargs.arg = (unsigned int)start_args;
    uargs.user_sp = (unsigned int)((char *)t->stack_base + t->stack_size);
    uargs.tls_base = (unsigned int)t->tls_base;

    int result = thread_create((void *)&uargs);
    if (result < 0) {
        mem_free(start_args);
        mem_free(t->tls_base);
        munmap(t->stack_base);
        mem_free(t);
        return -1;
    }

    t->tid = (tid_t)result;
    if (thread != NULL) {
        *thread = t->tid;
    }
    array_tid[ForkSelf()][t->tid] = t;

    return 0;
}

void pthread_exit(void *retval){
    pthread_lib* self = __pthread_self();

    self->retval = retval;
    self->exited = 1;

    atomic_store(&self->futex, 1);
    futex_wake(&self->futex, INT32_MAX);

    if (self->detached) {
        pthread_destroy(self->tid);
    }

    thread_exit();
}

int pthread_join(pthread_t thread, void **retval){
    GET_PTR_AND_CATCH_NULL_RETUN0(thread)
    if (thread_ptr->detached) {
        return -1;
    }

    while (atomic_load(&thread_ptr->futex) == 0) {
        futex_wait(&thread_ptr->futex, 0);
    }

    if (retval != NULL) {
        *retval = thread_ptr->retval;
    }

    pthread_destroy(thread);
    return 0;
}

int pthread_detach(pthread_t thread){
    GET_PTR_AND_CATCH_NULL_RETUN0(thread)
    if (thread_ptr->detached) {
        return -1;
    }
    thread_ptr->detached = 1;

    if (thread_ptr->exited) {
        pthread_destroy(thread);
    }

    return 0;
}

void pthread_destroy(pthread_t thread){
    GET_PTR_AND_CATCH_NULL(thread)
    munmap(thread_ptr->stack_base);
    mem_free(thread_ptr->tls_base);
    mem_free(thread_ptr);
    array_tid[ForkSelf()][thread] = NULL;
}

pthread_t pthread_self(){
    pthread_lib* thread = __pthread_self();
    return thread ? thread->tid : -1;
}
