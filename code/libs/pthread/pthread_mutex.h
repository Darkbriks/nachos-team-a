#ifndef PTHREAD_MUTEX_H
#define PTHREAD_MUTEX_H

#include "syscall.h"

// TODO: Premiére ébauche pour tester les futexs
//       A améliorer pour mieux coller a la spéc POSIX

typedef struct {
    int state;
} pthread_mutex_t;

int pthread_mutex_init(pthread_mutex_t* m) {
    m->state = 0;
    return 0;
}

/**
 * @brief Lock a mutex
 *
 * If the address isn't a valid mutex, the behavior is undefined.
 *
 * @param m The mutex to lock
 * @return 0 on success, -1 on error (check errno)
 */
int pthread_mutex_lock(pthread_mutex_t* m) {
    while (1) {
        //if (__atomic_compare_exchange_n(&m->state, &expected, 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        if (atomic_cmpxchg(&m->state, 0, 1) == 0) {
            return 0;
        }
        futex_wait(&m->state, 1);
    }
}

int pthread_mutex_unlock(pthread_mutex_t* m) {
    //__atomic_store_n(&m->state, 0, __ATOMIC_RELEASE);
    atomic_store(&m->state, 0);
    futex_wake(&m->state, 1);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t* m) {
    //if (__atomic_compare_exchange_n(&m->state, &expected, 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
    if (atomic_cmpxchg(&m->state, 0, 1) == 0) {
        return 0;
    }
    return -E_BUSY;
}

int pthread_mutex_destroy(pthread_mutex_t* m) {
    return 0;
}

#endif