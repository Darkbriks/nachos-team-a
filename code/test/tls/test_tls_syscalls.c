#include "syscall.h"
#include "tls.h"

int main(void) {
    tls_t my_tls;
    void *result;
    int ret, err;

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

    PutString("\nTest 1: GetTLS before set\n", 35);
    result = GetTLS();
    PutString("Result: ", 10);
    PutInt((int)result);
    if (result == (void*)0) {
        PutString(" OK\n", 10);
    } else {
        PutString(" UNEXPECTED\n", 20);
    }

    PutString("\nTest 2: SetTLS with valid pointer\n", 45);
    ret = SetTLS(&my_tls);
    err = GetLastError();
    PutString("Return value: ", 20);
    PutInt(ret);
    if (ret == 0) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 15);
        PutString("errno: ", 10);
        PutInt(err);
        PutChar('\n');
    }

    PutString("\nTest 3: GetTLS after set\n", 35);
    result = GetTLS();
    PutString("Result: ", 10);
    PutInt((int)result);
    PutString(", Expected: ", 15);
    PutInt((int)&my_tls);
    if (result == &my_tls) {
        PutString(" OK\n", 10);
    } else {
        PutString(" MISMATCH\n", 20);
    }

    PutString("\nTest 4: Read TLS via __get_tls() macro\n", 45);
    tls_t *tls_from_reg = __get_tls();
    PutString("$gp value: ", 15);
    PutInt((int)tls_from_reg);
    if (tls_from_reg == &my_tls) {
        PutString(" OK\n", 10);
    } else {
        PutString(" MISMATCH\n", 20);
    }

    PutString("\nTest 5: Self-pointer access\n", 35);
    if (tls_from_reg != (void*)0 && tls_from_reg->self == tls_from_reg) {
        PutString("self->self matches OK\n", 30);
    } else {
        PutString("self->self MISMATCH\n", 25);
    }

    PutString("\nTest 6: GetTID\n", 25);
    int tid = GetTID();
    PutString("Kernel TID: ", 15);
    PutInt(tid);
    PutChar('\n');
    if (tid >= 0) {
        PutString("OK\n", 20);
    } else {
        PutString("FAILED\n", 30);
    }

    PutString("\nTest 7: SetTLS with NULL\n", 35);
    ret = SetTLS((void*)0);
    err = GetLastError();
    PutString("Return value: ", 20);
    PutInt(ret);
    if (ret == -1) {
        PutString(" OK\n", 10);
        PutString("errno: ", 10);
        PutInt(err);
        PutChar('\n');
    } else {
        PutString(" FAILED\n", 10);
    }

    result = GetTLS();
    if (result == &my_tls) {
        PutString("TLS unchanged after failed call (OK)\n", 40);
    } else {
        PutString("TLS was modified! (BUG)\n", 30);
    }

    PutString("\n=== TLS Syscall Test Complete ===\n", 40);

    return 0;
}