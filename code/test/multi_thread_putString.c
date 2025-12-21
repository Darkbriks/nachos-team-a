#include "syscall.h"
#include "my_stdlib.h"

// Don't work
void very_long_function(void *arg) {
    my_printf("b");
    my_printf("Thread woke up from very long sleep!\n");
    Pthread_exit(0);
}

int main() {
    my_printf("c");

    posix_thread_t tid;
    Pthread_create(&tid, 0, (void *(*)(void *)) &very_long_function, 0);
    Pthread_join(tid, 0);

    my_printf("Main thread finished.\n");
    return 0;
}

