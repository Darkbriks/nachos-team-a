#include "nos_pthread_attr.h"
#include "nos_string.h"

#define PTHREAD_ATTR_SET_FLAG(name, _flag)  \
    int pthread_attr_set##name(pthread_attr_t * attr, int value){  \
        if (value == 0 || value == 1){                             \
            attr->flag &= ~_flag(1);                               \
            attr->flag |= _flag(value);                            \
            return 0;                                              \
        }                                                          \
        return 1;                                                  \
    }

#define PTHREAD_ATTR_GET_FLAG(name, _flag)  \
    int pthread_attr_get##name(pthread_attr_t * attr, int *value){  \
        *value = (attr->flag & _flag(1)) != 0  ? 1 : 0;             \
        return 0;                                                   \
    }

int pthread_attr_init(pthread_attr_t *attr){
    memset(attr, 0, sizeof(pthread_attr_t));
    attr->guard_size = PAGE_SIZE;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr){
    return 0;
}


PTHREAD_ATTR_SET_FLAG(detachstate,      DETACH_STATE)
PTHREAD_ATTR_SET_FLAG(scope,            SCOPE)
PTHREAD_ATTR_SET_FLAG(inheritscheduler, INHERIT_SCHEDULER)
PTHREAD_ATTR_SET_FLAG(schedulingpolicy, SCHEDULING_POLICY)

PTHREAD_ATTR_GET_FLAG(detachstate,      DETACH_STATE)
PTHREAD_ATTR_GET_FLAG(scope,            SCOPE)
PTHREAD_ATTR_GET_FLAG(inheritscheduler, INHERIT_SCHEDULER)
PTHREAD_ATTR_GET_FLAG(schedulingpolicy, SCHEDULING_POLICY)

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
