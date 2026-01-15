/* Test errno with tls in multithreaded environment */
#include "syscall.h"
#include "nos_errno.h"
#include "nos_mem_space.h"
#include "nos_stdlib.h"
#include "nos_stddef.h"
#include "nos_pthread.h"

#define NUM_THREADS 20

void thread_function(int thread_id) {

    int test_errno_value = thread_id;
    __set_errno(test_errno_value);

    // Simulate some processing
    for (volatile int i = 0; i < 10000; i++);

    int retrieved_errno = errno;
    if (retrieved_errno == test_errno_value) {
        PutString("Thread ", 8);
        PutInt(thread_id);
        PutString(": OK, errno = ", 14);
        PutInt(retrieved_errno);
        PutChar('\n');
    } else {
        PutString("Thread ", 8);
        PutInt(thread_id);
        PutString(": FAILED, expected errno = ", 26);
        PutInt(test_errno_value);
        PutString(", got errno = ", 15);
        PutInt(retrieved_errno);
        PutChar('\n');
    }
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    PutString("=== Errno Multithreaded Test ===\n", 35);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, (void*(*)(void*))thread_function, (void*)(long)i) != 0) {
            print_error("Failed to create thread "); PutInt(i); PutChar('\n');
            return -1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    PutString("=== Errno Multithreaded Test Complete ===\n", 45);
    return 0;
}
