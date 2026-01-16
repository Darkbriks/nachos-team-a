#include "syscall.h"
#include "nos_mem.h"
#include "nos_stdlib.h"
#include "nos_stdio.h"

void * hack(void * not_use){
    printf("coucou\n");
    Halt();
    return NULL;
}

int main(){
    void * addr = mem_init(100);
    char * addr_te = (( char *) addr);
    for (int i = 0; i< 80; i+=4){
        addr_te += 4;
        * (int **)addr_te =  (int *)hack;
        mem_alloc(10); // devrait créer un shell
    }
    return 0;
}

