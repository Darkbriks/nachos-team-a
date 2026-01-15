#ifndef NOS_ERRNO_H
#define NOS_ERRNO_H

#include "nos_tls.h"

/* ============================================================
 * ERROR CODES
 * Returned as negative values by syscalls, stored as positive in errno
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/errno.html">Documentation</a>
 * ============================================================
 */
#define E_SUCCESS       0   /* No error */
#define E_INVAL         1   /* Invalid argument */
#define E_FAULT         2   /* Bad address / memory access error */
#define E_OVERFLOW      3   /* Arithmetic overflow */
#define E_IO            4   /* I/O error */
#define E_FORMAT        5   /* Invalid format */
#define E_EOF           6   /* End of file */
#define E_NOMEM         7   /* Out of memory */
#define E_RANGE         8   /* Result out of range */
#define E_NOSPC         9   /* No such process */
#define E_FTABLE        10  /* Allocation table full (file table, semaphore table, etc.) */
#define E_NOENT         11  /* No such file or directory or table entry */
#define E_NOCPC         12  /* Not a child process */
#define E_THREAD_LIMIT  13  /* Maximum threads reached */
#define E_STACK_ADDR    14  /* Invalid stack address */
#define E_BUSY          15  /* Resource busy */
#define E_AGAIN         16  /* Resource temporarily unavailable */
#define E_DOM           17  /* Math argument out of domain */
#define E_ILSEQ         18  /* Illegal byte sequence */
#define E_PERM          19  /* Operation not permitted */
#define E_ACCES         20  /* Permission denied */
#define E_EXIST         21  /* File exists */
#define E_NOSYS         22  /* Function not implemented */
#define E_NOTDIR        23  /* Not a directory */
#define E_ISDIR         24  /* Is a directory */
#define E_BADF          25  /* Bad file descriptor */


/* ============================================================
 * Thread-local errno implementation
 * ============================================================
 */

/**
 * @brief Global errno variable.
 *
 * This variable serves as a fallback for the errno variable when
 * thread-local storage (TLS) is not available. It is used to store
 * error codes in a global context.
 */
extern int __global_errno;

#ifdef IN_USER_MODE

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

#endif

#endif