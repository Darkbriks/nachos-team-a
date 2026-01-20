#ifndef PTHREAD_MUTEX_H
#define PTHREAD_MUTEX_H

#include "nos_errno.h"

#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_ERRORCHECK 1
#define PTHREAD_MUTEX_RECURSIVE  2
#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL

#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1  // Not implemented yet
#define PTHREAD_PRIO_PROTECT 2  // Not implemented yet

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED  1  // Not implemented yet

#define PTHREAD_MUTEX_STALLED 0
#define PTHREAD_MUTEX_ROBUST  1  // Not implemented yet

typedef struct {
    int type;        // PTHREAD_MUTEX_NORMAL, ERRORCHECK, RECURSIVE, DEFAULT
    int protocol;    // PTHREAD_PRIO_NONE, INHERIT, PROTECT
    int pshared;     // PTHREAD_PROCESS_PRIVATE or SHARED
    int robust;      // PTHREAD_MUTEX_STALLED or ROBUST
    int prioceiling; // Priority ceiling (for PTHREAD_PRIO_PROTECT)
} pthread_mutexattr_t;

typedef struct {
    int state;           // 0 = unlocked, 1 = locked
    int type;            // Type of mutex (NORMAL, ERRORCHECK, RECURSIVE)
    int owner;           // Thread ID of owner (-1 if unlocked)
    unsigned int count;  // Lock count (for RECURSIVE mutex)
    int protocol;        // Priority protocol (currently unused)
    int robust;          // Robustness flag (currently unused)
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { 0, PTHREAD_MUTEX_DEFAULT, 0, 0, PTHREAD_PRIO_NONE, PTHREAD_MUTEX_STALLED }

#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP { 0, PTHREAD_MUTEX_ERRORCHECK, 0, 0, PTHREAD_PRIO_NONE, PTHREAD_MUTEX_STALLED }

#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP { 0, PTHREAD_MUTEX_RECURSIVE, 0, 0, PTHREAD_PRIO_NONE, PTHREAD_MUTEX_STALLED }

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling);

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling);
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);
int pthread_mutex_consistent(pthread_mutex_t *mutex);

#endif