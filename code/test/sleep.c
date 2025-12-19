#include "syscall.h"
#include "my_stdlib.h"

// Work
/*int main() {
    long long before_sleep;
    long long after_sleep;

    my_printf("=== Test Sleep syscall ===\n");
    GetCurrentTick(&before_sleep);
    Sleep(5000);
    GetCurrentTick(&after_sleep);
    my_printf("Main: Woke up after 5000 ticks\n");

    my_printf("Main: Slept for ");
    PutInt(after_sleep - before_sleep);
    my_printf(" ticks (expected ~5000)\n");

    return 0;
}*/

// Don't work
void very_long_function(void *arg) {
    my_printf("Thread started, going to sleep for a very long time...\n");
    Sleep(1000); // Sleep for a long time
    my_printf("Thread woke up from very long sleep!\n");
    ExitThread();
}

int main() {
    my_printf("=== Test Sleep syscall with very long sleep ===\n");

    JoinThread(CreateThread(very_long_function, nullptr));

    my_printf("Main thread finished.\n");
    return 0;
}