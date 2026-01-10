#ifndef NOS_ERRNO_H
#define NOS_ERRNO_H

#include "tls.h"

extern int __global_errno;

static inline int* __errno_location() {
  tls_t* tls = __get_tls();
  if (tls != (tls_t*)0) {
    return &tls->errno_val;
  }
  return &__global_errno;
}

#define errno (*__errno_location())

static inline void __set_errno(const int value) {
  *__errno_location() = value;
}

static inline int __get_errno() {
  return *__errno_location();
}

static inline void __clear_errno() {
  *__errno_location() = 0;
}

#endif