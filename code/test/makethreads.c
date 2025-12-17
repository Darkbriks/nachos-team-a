#include "syscall.h"

void super_fun_2(void *arg){
    ExitThread();
}

void super_fun_1(void *arg){
    for (int i = 0; i < *(int *)arg; i++) { CreateThread(super_fun_2, arg); }
    ExitThread();
}

int main(){
    int x = 3;
    for (int i = 0; i < x; i++) { CreateThread(super_fun_1, &x); }
    return 0;
}
