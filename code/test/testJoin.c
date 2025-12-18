
#include "syscall.h"
char *ma_chaine = "abcdefghijklmnopqrstuvwxyz";

void super_fun_2(void *arg){
    PutChar('d');
    for (int i= 0; i< 55555; i++){}
    PutChar('e');
    ExitThread();
}

void super_fun_1(void *arg){
    PutChar('a');
    int tid = CreateThread(super_fun_2, (void *) 0);
    JoinThread(tid);
    PutChar('b');
    PutChar('c');
    ExitThread();
}

int main(){
    CreateThread(super_fun_1, (void *) 0);
}

