#include "syscall.h"

void super_fun_2(void *arg) {
    PutString("f2 : arg value = ", 16); PutInt(*(int *) arg); PutChar('\n');
}

void super_fun_1(void *arg) {
    int tmp = *(int *) arg;
    PutString("f1 : arg value = ", 16); PutInt(tmp); PutChar('\n');

    for (int i = 0; i < tmp + 1; i++) {
        int offset = i * 4;
        PthreadCreate(0, 0, (void *(*)(void *)) &super_fun_2, (void *) (arg + offset));
    }
}

int main() {
    int x = 3;
    int my_tab[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    for (int i = 0; i < x; i++) { PthreadCreate(0, 0, (void *(*)(void *)) &super_fun_1, &my_tab[i]); }
}

