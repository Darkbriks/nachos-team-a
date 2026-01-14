#include "syscall.h"
#include "nos_stdlib.h"
#include "nos_errno.h"
#include "nos_stdio.h"

int main() {
    long long current_tick;

    printf("=== Test SleepUntil syscall : simple sleep until ===\n");
    SleepUntil(10000);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick "); PutInt(current_tick); printf(" (expected >= 10000)\n");

    printf("=== Test SleepUntil syscall : long sleep until ===\n");
    SleepUntil(1000000);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick "); PutInt(current_tick); printf(" (expected >= 1000000)\n");

    printf("=== Test SleepUntil syscall : past time sleep until ===\n");
    SleepUntil(500);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick "); PutInt(current_tick); printf(" (expected >= previous tick)\n");

    printf("=== Test SleepUntil syscall : invalid sleep until (negative time) ===\n");
    int errcode;
    int result = SleepUntil(-100);
    if (result == -1) {
        errcode = __get_errno();
        printf("Main: SleepUntil(-100) failed as expected with errno = ");
        PutInt(errcode);
        printf("\n");
    } else {
        printf("Main: SleepUntil(-100) unexpectedly succeeded\n");
    }

    return 0;
}
