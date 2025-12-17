#include "syscall.h"

void super_fun(void *arg){
    PutChar('h');
    ExitThread();
}

int main(){
    int x = 8;
    CreateThread(super_fun, &x);
    ExitThread(); 
    return 0;
}
