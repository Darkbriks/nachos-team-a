#include "syscall.h"
#include "nos_errno.h"
#include "nos_mem_space.h"
#include "nos_stdlib.h"

int main(void) {
    tls_t my_tls;
    int err;

    my_tls.self = &my_tls;
    my_tls.tid = 0;
    my_tls.pid = 0;
    my_tls.errno_val = 0;
    my_tls.stack_base = (void*)0;
    my_tls.stack_size = 0;
    my_tls.flags = TLS_FLAG_JOINABLE;
    my_tls.retval = (void*)0;

    PutString("TLS block address: ", 25);
    PutInt((int)&my_tls);
    PutChar('\n');

    if (SetTLS(&my_tls) != 0) {
        print_error("Failed to set TLS");
        return -1;
    }

    PutString("\nTest 0: errno address != __global_errno address\n", 50);
    int* errno_addr = &errno;
    int* global_errno_addr = &__global_errno;
    PutString("errno address: ", 17);
    PutInt((int)errno_addr);
    PutChar('\n');
    PutString("__global_errno address: ", 25);
    PutInt((int)global_errno_addr);
    PutChar('\n');
    if (errno_addr != global_errno_addr) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 1: Initial errno\n", 30);
    err = errno;
    PutString("errno = ", 10);
    PutInt(err);
    if (err == 0) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 2: Manualy set errno to E_INVAL\n", 40);
    errno = E_INVAL;
    err = errno;
    int gerrno = __global_errno;
    PutString("errno after setting to E_INVAL = ", 35);
    PutInt(err);
    if (err == E_INVAL) {
        PutString(" OK\n", 10);
        PutString("\nTest 2b: Check __global_errno remains 0\n", 40);
        PutString("__global_errno = ", 20);
        PutInt(gerrno);
        if (gerrno == 0) {
            PutString(" OK\n", 10);
        } else {
            PutString(" FAILED\n", 10);
        }
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 3: Trigger error (SemWait on invalid id)\n", 50);
    int sem_ret = SemWait(-1);  /* Invalid semaphore ID */
    err = errno;
    PutString("SemWait(-1) returned: ", 25);
    PutInt(sem_ret);
    PutChar('\n');
    PutString("errno = ", 10);
    PutInt(err);
    if (err == E_NOENT) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }
}