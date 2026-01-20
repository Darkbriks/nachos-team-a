#ifndef NOS_UNISTD_H
#define NOS_UNISTD_H
#include "types.h"
#include "nos_bool.h"

#define O_CREATE 1
/* Close the file descriptor FD.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
bool close_file(int __fd);

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t read (int __fd, void *__buf, size_t __nbytes) ;

/* Write N bytes of BUF to FD.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t write (int __fd, const void *__buf, size_t __n) ;

int open(char* name, int flags);

bool create(char *name);
#endif // NOS_UNISTD_H
