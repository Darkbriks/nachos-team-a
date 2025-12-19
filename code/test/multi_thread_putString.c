#include "syscall.h"
#include "my_stdlib.h"

// Don't work
void very_long_function(void *arg) {
    my_printf("b");
    my_printf("Thread woke up from very long sleep!\n");
    ExitThread();
}

int main() {
    my_printf("c");

    JoinThread(CreateThread(very_long_function, 0));

    my_printf("Main thread finished.\n");
    return 0;
}

