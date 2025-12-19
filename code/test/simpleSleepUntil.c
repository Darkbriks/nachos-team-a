#include "syscall.h"
#include "my_stdlib.h"

int main() {
    long long current_tick;

    my_printf("=== Test SleepUntil syscall : simple sleep until ===\n");
    SleepUntil(10000);
    GetCurrentTick(&current_tick);
    my_printf("Main: Woke up at tick "); PutInt(current_tick); my_printf(" (expected >= 10000)\n");

    my_printf("=== Test SleepUntil syscall : long sleep until ===\n");
    SleepUntil(1000000);
    GetCurrentTick(&current_tick);
    my_printf("Main: Woke up at tick "); PutInt(current_tick); my_printf(" (expected >= 1000000)\n");

    my_printf("=== Test SleepUntil syscall : past time sleep until ===\n");
    SleepUntil(500);
    GetCurrentTick(&current_tick);
    my_printf("Main: Woke up at tick "); PutInt(current_tick); my_printf(" (expected >= previous tick)\n");

    my_printf("=== Test SleepUntil syscall : invalid sleep until (negative time) ===\n");
    int errcode;
    int result = SleepUntil(-100);
    if (result == -1) {
        errcode = GetLastError();
        my_printf("Main: SleepUntil(-100) failed as expected with errno = ");
        PutInt(errcode);
        my_printf("\n");
    } else {
        my_printf("Main: SleepUntil(-100) unexpectedly succeeded\n");
    }

    return 0;
}