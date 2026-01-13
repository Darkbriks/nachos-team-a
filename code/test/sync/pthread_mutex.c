#include "pthread_mutex.h"
#include "syscall.h"
#include "nos_stddef.h"
#include "nos_threads.h"
#include "pthread.h"

pthread_mutex_t mutex;
int shared_counter = 0;
const int NUM_THREADS = 5;
const int INCREMENTS_PER_THREAD = 1000;

void* thread_function(void* arg) {
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex);

    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int expected_value = NUM_THREADS * INCREMENTS_PER_THREAD;
    if (shared_counter == expected_value) {
        PutString("Test passed: shared_counter = ", 35);
        PutInt(shared_counter);
        PutString("\n", 1);
    } else {
        PutString("Test failed: expected ", 35);
        PutInt(expected_value);
        PutString(", got ", 6);
        PutInt(shared_counter);
        PutString("\n", 1);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
