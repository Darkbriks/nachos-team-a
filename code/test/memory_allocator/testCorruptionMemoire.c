#include "nos_mem.h"

int main(){
    mem_init(100);
    char *x = mem_alloc(16); 
    for (int i = 0; i < 100; i++){
        *(x-i) = 1;
    }
    // TODO ne plius effacer le .code du fichier 
    mem_free(x);
    return 0;
}


