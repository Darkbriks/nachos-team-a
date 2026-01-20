#include "syscall.h"
#include "nos_errno.h"
#include "nos_stdio.h"

int main() {
    long long current_tick;
    long long start_program_tick;
    GetCurrentTick(&start_program_tick);

    printf("=== Test SleepUntil syscall : simple sleep until ===\n");
    SleepUntil(start_program_tick + 10000);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick %i (expected >= %i)\n", (int)(current_tick - start_program_tick), (int)(10000));

    printf("=== Test SleepUntil syscall : long sleep until ===\n");
    SleepUntil(start_program_tick + 1000000);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick %i (expected >= %i)\n", (int)(current_tick - start_program_tick), (int)(1000000));

    printf("=== Test SleepUntil syscall : past time sleep until ===\n");
    SleepUntil(start_program_tick);
    GetCurrentTick(&current_tick);
    printf("Main: Woke up at tick %i (expected >= %i)\n", (int)(current_tick - start_program_tick), (int)(start_program_tick));

    printf("=== Test SleepUntil syscall : invalid sleep until (negative time) ===\n");
    int errcode;
    int result = SleepUntil(-100);
    if (result == -1) {
        errcode = __get_errno();
        printf("Main: SleepUntil(-100) failed as expected with errno = %i\n", errcode);
    } else {
        printf("Main: SleepUntil(-100) unexpectedly succeeded\n");
    }

    return 0;
}
