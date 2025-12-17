#include "syscall.h"

void super_fun(void *arg){
    PutChar('1');
    PutInt(*(int *)arg);
    ExitThread();
    PutChar('2');
}

int main(){
    int x = 8;
    CreateThread(super_fun, &x);
    ExitThread();
    return 0;
}
