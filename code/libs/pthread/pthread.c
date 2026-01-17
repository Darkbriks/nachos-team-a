#include "pthread.h"
#include "tls.h"
#include "syscall.h"
#include "nos_threads.h"
#include "nos_string.h"
#include "nos_stddef.h"
#include "nos_mem.h"
#include "nos_mem_macros.h"
#include "nos_limits.h"
#include "types.h"

int mem_alloc_init = 0;

static inline unsigned int calculate_initial_mem_size() {
    const unsigned int threads_estimate = 4;
    const unsigned int per_thread_size = DEFAULT_STACK_SIZE + sizeof(tls_t) + sizeof(pthread_lib) + 512;
    const unsigned int base_overhead = INIT_BLOCK_SIZE + FREE_BLOCK_TOTAL(0) + 1024;
    return threads_estimate * per_thread_size + base_overhead;
}
int mem_init_amount = 0;

pthread_lib* array_tid[MAX_PROCESS][MAX_THREAD];

#define INIT_MEMORY_ALLOCATOR()    \
    if (mem_alloc_init == 0) {     \
        mem_alloc_init = 1;        \
        if (mem_init_amount == 0) { mem_init_amount = calculate_initial_mem_size(); } \
        mem_init(mem_init_amount); \
    }

#define GET_PTR_AND_CATCH_NULL_RETUN0(TID)    \
    pthread_lib* thread_ptr = get_thread_by_tid(TID);  \
    if (! thread_ptr){                                 \
        return -1;                                     \
    }
                                                        
#define GET_PTR_AND_CATCH_NULL(TID)    \
    pthread_lib* thread_ptr = get_thread_by_tid(TID);  \
    if (! thread_ptr){                                 \
        return;                                        \
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

static inline pthread_lib* __pthread_self(){
    return (pthread_lib*)__get_tls()->pthread_ptr;
}

static inline pthread_lib* get_thread_by_tid(pthread_t thread){
    return array_tid[ForkSelf()][thread];
}

int init_pthread_lib(pthread_lib* t, char detach, int size){
    t->exited = 0;
    t->detached = detach;
    t->futex = 0;
    t->retval = NULL;
    t->stack_size = size;
    t->pid = ForkSelf();

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
    return 0;
}

void init_tls(pthread_lib* t){
    tls_t* tls = (tls_t*)t->tls_base;
    tls->self = tls;
    tls->errno_val = 0;
    tls->pthread_ptr = (int)t;
}

struct thread_start_args* create_start_arg(typeof(void *(void *)) *fun, void * arg){
    struct thread_start_args* result = mem_alloc(sizeof(struct thread_start_args));
    if (result == NULL) {
        return result;
    }
    result->start_routine = fun;
    result->arg = arg;
    return result;
}

int try_to_launch_thread(pthread_lib* t, struct thread_start_args* start_args){
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
    return 0;
}

int pthread_create(pthread_t *thread, pthread_attr_t * attr, typeof(void *(void *)) *fun, void * arg) {
    INIT_MEMORY_ALLOCATOR();

    pthread_lib* t = (pthread_lib*) mem_alloc(sizeof(pthread_lib));
    if (t == NULL) {
        return -1;
    }

    char detacheState = attr ? (attr->flag & DETACH_STATE(DETACHED)) != 0  ? 1 : 0 : 0; 
    int stack_size = attr && attr->stack_size > 0 ? attr->stack_size : DEFAULT_STACK_SIZE; 
    if ( init_pthread_lib(t, detacheState, stack_size) != 0){
        return -1;
    }

    init_tls(t);

    struct thread_start_args* start_args;
    if ( (start_args = create_start_arg(fun, arg)) == NULL){
        mem_free(t->tls_base);
        munmap(t->stack_base);
        mem_free(t);
        return -1;
    }

    if (  try_to_launch_thread(t, start_args) != 0){
        return -1;
    }

    if (thread != NULL) {
        *thread = t->tid;
    }
    array_tid[ForkSelf()][t->tid] = t;

    return 0;
}

void pthread_exit(void *retval){
    pthread_lib* self = __pthread_self();
    if (! self){ // thread don't create with pthread lib but with syscall or first of process
        thread_exit();
        return;
    }

    self->retval = retval;
    self->exited = 1;

    atomic_store(&self->futex, 1);
    futex_wake(&self->futex, INT_MAX);

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
