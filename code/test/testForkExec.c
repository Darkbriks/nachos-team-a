#include "syscall.h"
#include "nos_stddef.h"

int main(){
    for (int i = 0; i < 12; i++){
        ForkJoin(ForkExec("./userpages0", 15), NULL);
        ForkJoin(ForkExec("./userpages1", 15), NULL);
    }
    PutString("All processes launched.\n", 26);
    return 0;
}
