/* Test errno with tls in multithreaded environment */
#include "syscall.h"
#include "nos_errno.h"
#include "nos_mem_space.h"
#include "nos_stdlib.h"

#define NUM_THREADS 20

void thread_function(int thread_id) {
    tls_t my_tls;
    my_tls.self = &my_tls;
    my_tls.tid = thread_id;
    my_tls.pid = 0;
    my_tls.errno_val = 0;
    my_tls.stack_base = (void*)0;
    my_tls.stack_size = 0;
    my_tls.flags = TLS_FLAG_JOINABLE;
    my_tls.retval = (void*)0;

    if (SetTLS(&my_tls) != 0) {
        print_error("Thread "); PutInt(thread_id); PutString(": Failed to set TLS\n", 20);
        return;
    }

    int test_errno_value = thread_id;
    errno = test_errno_value;

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
    posix_thread_t threads[NUM_THREADS];

    PutString("=== Errno Multithreaded Test ===\n", 35);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (PthreadCreate(&threads[i], NULL, (void*(*)(void*))thread_function, (void*)(long)i) != 0) {
            print_error("Failed to create thread "); PutInt(i); PutChar('\n');
            return -1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        PthreadJoin(threads[i], NULL);
    }

    PutString("=== Errno Multithreaded Test Complete ===\n", 45);
    return 0;
}