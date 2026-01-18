#include "syscall.h"
#include "pthread.h"
#include "pthread_mutex.h"

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void super_fun_2(void *arg) {
    pthread_mutex_lock(&print_mutex);
    PutString("f2 : arg value = ", 16); PutInt(*(int *) arg); PutChar('\n');
    pthread_mutex_unlock(&print_mutex);
}

void super_fun_1(void *arg) {
    int tmp = *(int *) arg;
    pthread_mutex_lock(&print_mutex);
    PutString("f1 : arg value = ", 16); PutInt(tmp); PutChar('\n');
    pthread_mutex_unlock(&print_mutex);

    for (int i = 0; i < tmp + 1; i++) {
        int offset = i * 4;
        pthread_create(0, 0, (void *(*)(void *)) &super_fun_2, (void *) (arg + offset));
    }
}

int main() {
    int x = 3;
    int my_tab[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    for (int i = 0; i < x; i++) { pthread_create(0, 0, (void *(*)(void *)) &super_fun_1, &my_tab[i]); }

    pthread_mutex_lock(&print_mutex);
    PutString("Main thread exiting\n", 20);
    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_destroy(&print_mutex);
    return 0;
}