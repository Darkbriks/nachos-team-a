#include "syscall.h"
#include "nos_errno.h"

int main(void) {
    int err;

    err = __get_errno();

    PutString("=== Errno Basic Test ===\n", 40);

    PutString("\nTest 1: Initial errno\n", 30);
    PutString("errno = ", 10);
    PutInt(err);
    if (err == 0) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 2: ClearError\n", 25);
    __clear_errno();
    err = __get_errno();
    PutString("errno after ClearError = ", 30);
    PutInt(err);
    if (err == 0) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 3: Trigger error (SemWait on invalid id)\n", 50);
    int ret = SemWait(-1);  /* Invalid semaphore ID */
    err = __get_errno();
    PutString("SemWait(-1) returned: ", 25);
    PutInt(ret);
    PutChar('\n');
    PutString("errno = ", 10);
    PutInt(err);
    if (err == E_NOENT) {
        PutString(" OK\n", 10);
    } else {
        PutString(" FAILED\n", 10);
    }

    PutString("\nTest 4: Successful call clears errno\n", 45);
    PutChar('X');
    err = __get_errno();
    PutString("\nerrno after PutChar = ", 25);
    PutInt(err);

    PutString("\n=== Errno Basic Test Complete ===\n", 40);

    return 0;
}