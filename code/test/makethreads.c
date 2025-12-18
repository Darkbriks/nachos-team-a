#include "syscall.h"
char *ma_chaine = "abcdefghijklmnopqrstuvwxyz";

void super_fun_2(void *arg){
    PutInt((int) arg);
    PutChar('\n');
    ExitThread();
}

void super_fun_1(void *arg){
    int *tmp = (int *) arg;
    PutInt((int) arg);
    PutChar('\n');
    PutInt(*tmp);
    PutChar('\n');
    for (int i = 0; i < *tmp; i++) { 
        int offset = i *4;
        CreateThread(super_fun_2, (void *) (arg + offset));
    }
    ExitThread();
}

int main(){
    int x = 3;
    int my_tab[] = {0,1,2,4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,8192, 16384, 32768, 65536};
    for (int i = 0; i < x; i++) { CreateThread(super_fun_1, (void *) &(my_tab[i])); }
}

