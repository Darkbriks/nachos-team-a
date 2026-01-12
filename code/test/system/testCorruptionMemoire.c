#include "nos_mem.h"

int main(){
    mem_init(100);
    char *x = mem_alloc(16); 
    for (int i = 0; i < 100317; i++){
        *(x+i) = 1;
    }
    mem_free(x);
    return 0;
}


