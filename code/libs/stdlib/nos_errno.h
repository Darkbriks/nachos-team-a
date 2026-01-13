#ifndef NOS_ERRNO_H
#define NOS_ERRNO_H

#include "tls.h"

/**
 * @brief Global errno variable.
 *
 * This variable serves as a fallback for the errno variable when
 * thread-local storage (TLS) is not available. It is used to store
 * error codes in a global context.
 */
extern int __global_errno;

/**
 * @brief Get the address of the thread-local errno variable.
 *
 * This function retrieves the address of the errno variable specific to the
 * current thread. If thread-local storage (TLS) is available, it returns the
 * address of the errno variable within the TLS structure. If TLS is not
 * available, it falls back to a global errno variable.
 *
 * @return Pointer to the thread-local errno variable, or global errno if TLS is unavailable.
 */
static inline int* __errno_location() {
  tls_t* tls = __get_tls();
  if (tls != (tls_t*)0) {
    return &tls->errno_val;
  }
  return &__global_errno;
}

/**
 * @brief Thread-local errno variable.
 *
 * This macro provides access to the errno variable specific to the current
 * thread. It uses the __errno_location function to retrieve the address of
 * the thread-local errno variable.
 */
#define errno (*__errno_location())

/**
 * @brief Set the thread-local errno variable to a specified value.
 *
 * This function sets the errno variable specific to the current thread to
 * the provided value. It uses the __errno_location function to retrieve the
 * address of the thread-local errno variable.
 *
 * @param value The value to set errno to.
 */
static inline void __set_errno(const int value) {
  *__errno_location() = value;
}

/**
 * @brief Get the current value of the thread-local errno variable.
 *
 * This function retrieves the current value of the errno variable specific
 * to the current thread. It uses the __errno_location function to access
 * the thread-local errno variable.
 *
 * @return The current value of errno.
 */
static inline int __get_errno() {
  return *__errno_location();
}

/**
 * @brief Clear the thread-local errno variable (set to 0).
 *
 * This function sets the errno variable specific to the current thread to
 * 0, effectively clearing any previous error state. It uses the
 * __errno_location function to access the thread-local errno variable.
 */
static inline void __clear_errno() {
  *__errno_location() = 0;
}

// TODO: Define standard errno values here instead of in syscall.h
// https://en.cppreference.com/w/cpp/header/cerrno.html

#endif