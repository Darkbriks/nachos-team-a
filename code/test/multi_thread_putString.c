#include "syscall.h"
#include "my_stdlib.h"

// Don't work
void very_long_function(void *arg) {
    my_printf("b");
    my_printf("Thread woke up from very long sleep!\n");
    PthreadExit(0);
}

int main() {
    my_printf("c");

    posix_thread_t tid;
    PthreadCreate(&tid, 0, (void *(*)(void *)) &very_long_function, 0);
    PthreadJoin(tid, 0);

    my_printf("Main thread finished.\n");
    return 0;
}

