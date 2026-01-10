#include "syscall.h"
#include "nos_stdlib.h"
#include "nos_errno.h"

int main() {
    long long current_tick;

    printf_simple("=== Test SleepUntil syscall : simple sleep until ===\n");
    SleepUntil(10000);
    GetCurrentTick(&current_tick);
    printf_simple("Main: Woke up at tick "); PutInt(current_tick); printf_simple(" (expected >= 10000)\n");

    printf_simple("=== Test SleepUntil syscall : long sleep until ===\n");
    SleepUntil(1000000);
    GetCurrentTick(&current_tick);
    printf_simple("Main: Woke up at tick "); PutInt(current_tick); printf_simple(" (expected >= 1000000)\n");

    printf_simple("=== Test SleepUntil syscall : past time sleep until ===\n");
    SleepUntil(500);
    GetCurrentTick(&current_tick);
    printf_simple("Main: Woke up at tick "); PutInt(current_tick); printf_simple(" (expected >= previous tick)\n");

    printf_simple("=== Test SleepUntil syscall : invalid sleep until (negative time) ===\n");
    int errcode;
    int result = SleepUntil(-100);
    if (result == -1) {
        errcode = __get_errno();
        printf_simple("Main: SleepUntil(-100) failed as expected with errno = ");
        PutInt(errcode);
        printf_simple("\n");
    } else {
        printf_simple("Main: SleepUntil(-100) unexpectedly succeeded\n");
    }

    return 0;
}