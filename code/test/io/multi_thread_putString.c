#include "syscall.h"
#include "nos_stdlib.h"

// Don't work
void very_long_function(void *arg) {
    printf_simple("b");
    printf_simple("Thread woke up from very long sleep!\n");
    PthreadExit(0);
}

int main() {
    printf_simple("c");

    posix_thread_t tid;
    PthreadCreate(&tid, 0, (void *(*)(void *)) &very_long_function, 0);
    PthreadJoin(tid, 0);

    printf_simple("Main thread finished.\n");
    return 0;
}

