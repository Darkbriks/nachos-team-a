#include "syscall.h"
#include "nos_stdlib.h"
#include "nos_threads.h"
#include "pthread.h"

// Don't work
void very_long_function(void *arg) {
    printf_simple("b");
    printf_simple("Thread woke up from very long sleep!\n");
    pthread_exit(0);
}

int main() {
    printf_simple("c");

    pthread_t tid;
    pthread_create(&tid, 0, (void *(*)(void *)) &very_long_function, 0);
    pthread_join(tid, 0);

    printf_simple("Main thread finished.\n");
    return 0;
}

