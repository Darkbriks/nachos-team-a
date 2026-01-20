#include "pthread_mutex.h"
#include "nos_string.h"
#include "syscall.h"

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
    if (attr == NULL) { return E_INVAL; }
    
    attr->type = PTHREAD_MUTEX_DEFAULT;
    attr->protocol = PTHREAD_PRIO_NONE;
    attr->pshared = PTHREAD_PROCESS_PRIVATE;
    attr->robust = PTHREAD_MUTEX_STALLED;
    attr->prioceiling = 0;
    
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
    if (attr == NULL) { return E_INVAL; }
    return 0;
}

#define PTHREAD_MUTEXATTR_GETTER(name)                                         \
int pthread_mutexattr_get##name(const pthread_mutexattr_t *attr, int *out) {   \
    if (attr == NULL || out == NULL) { return E_INVAL; }                        \
    *out = attr->name;                                                          \
    return 0;                                                                  \
}

PTHREAD_MUTEXATTR_GETTER(type)
PTHREAD_MUTEXATTR_GETTER(protocol)
PTHREAD_MUTEXATTR_GETTER(pshared)
PTHREAD_MUTEXATTR_GETTER(robust)
PTHREAD_MUTEXATTR_GETTER(prioceiling)

#define PTHREAD_MUTEXATTR_SETTER(name, valid_check)                            \
int pthread_mutexattr_set##name(pthread_mutexattr_t *attr, int value) {        \
    if (attr == NULL) { return E_INVAL; }                                       \
    if (!(valid_check)) { return E_INVAL; }                                     \
    attr->name = value;                                                        \
    return 0;                                                                  \
}

PTHREAD_MUTEXATTR_SETTER(type, (value == PTHREAD_MUTEX_NORMAL || \
                                 value == PTHREAD_MUTEX_ERRORCHECK || \
                                 value == PTHREAD_MUTEX_RECURSIVE || \
                                 value == PTHREAD_MUTEX_DEFAULT))

// Only PTHREAD_PRIO_NONE is supported currently
PTHREAD_MUTEXATTR_SETTER(protocol, (value == PTHREAD_PRIO_NONE || \
                                     value == PTHREAD_PRIO_INHERIT || \
                                     value == PTHREAD_PRIO_PROTECT))

// Only PTHREAD_PROCESS_PRIVATE is supported currently
PTHREAD_MUTEXATTR_SETTER(pshared, (value == PTHREAD_PROCESS_PRIVATE || \
                                   value == PTHREAD_PROCESS_SHARED))

// Only PTHREAD_MUTEX_STALLED is supported currently
PTHREAD_MUTEXATTR_SETTER(robust, (value == PTHREAD_MUTEX_STALLED || \
                                  value == PTHREAD_MUTEX_ROBUST))

PTHREAD_MUTEXATTR_SETTER(prioceiling, (value >= 0))

/* ============================================================
 * Mutex Functions
 * ============================================================ */

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    if (mutex == NULL) { return E_INVAL; }
    
    mutex->state = 0;
    mutex->owner = -1;
    mutex->count = 0;
    
    if (attr == NULL) {
        mutex->type = PTHREAD_MUTEX_DEFAULT;
        mutex->protocol = PTHREAD_PRIO_NONE;
        mutex->robust = PTHREAD_MUTEX_STALLED;
    } else {
        mutex->type = attr->type;
        mutex->protocol = attr->protocol;
        mutex->robust = attr->robust;
    }
    
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
    if (mutex == NULL) { return E_INVAL; }
    
    if (atomic_load(&mutex->state) != 0) {
        return E_BUSY;
    }
    
    mutex->state = 0;
    mutex->owner = -1;
    mutex->count = 0;
    
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    if (mutex == NULL) { return E_INVAL; }
    
    const int self = thread_self();
    
    // RECURSIVE mutex: allow re-locking by owner
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner == self) { mutex->count++; return 0; }
    }
    
    // ERRORCHECK mutex: detect deadlock
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner == self) { return E_DEADLK; }
    }
    
    while (1) {
        if (atomic_cmpxchg(&mutex->state, 0, 1) == 0) {
            mutex->owner = self;
            mutex->count = 1;
            return 0;
        }
        futex_wait(&mutex->state, 1);
    }
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
    if (mutex == NULL) { return E_INVAL; }
    
    const int self = thread_self();
    
    // RECURSIVE mutex: allow re-locking by owner
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner == self) { mutex->count++; return 0;  }
    }
    
    // ERRORCHECK mutex: detect deadlock
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner == self) { return E_DEADLK; }
    }
    
    if (atomic_cmpxchg(&mutex->state, 0, 1) == 0) {
        mutex->owner = self;
        mutex->count = 1;
        return 0;
    }
    
    return E_BUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    if (mutex == NULL) { return E_INVAL; }
    
    const int self = thread_self();
    
    // ERRORCHECK: verify ownership
    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK) {
        if (mutex->owner != self) { return E_PERM; }
    }
    
    // RECURSIVE: decrement count
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner != self) { return E_PERM; }
        mutex->count--;
        if (mutex->count > 0) { return 0; }
    }
    
    mutex->owner = -1;
    mutex->count = 0;
    atomic_store(&mutex->state, 0);
    futex_wake(&mutex->state, 1);
    
    return 0;
}

// Priority ceiling not implemented, No-op for now
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling) {
    if (mutex == NULL || prioceiling == NULL) { return E_INVAL; }
    *prioceiling = 0;
    return 0;
}

// Priority ceiling not implemented, No-op for now
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling) {
    if (mutex == NULL) { return E_INVAL; }
    if (prioceiling < 0) { return E_INVAL; }
    if (old_ceiling != NULL) { *old_ceiling = 0; }
    return 0;
}

// Robust mutexes not implemented, No-op for now
int pthread_mutex_consistent(pthread_mutex_t *mutex) {
    if (mutex == NULL) { return E_INVAL; }
    return 0;
}
