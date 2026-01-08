#include "syscall.h"
#include "types.h"

int main(){
    for (int i = 0; i < 12; i++){
        ForkJoin(ForkExec("./userpages0"), NULL);
        ForkJoin(ForkExec("./userpages1"), NULL);
    }
    PutString("All processes launched.\n", 26);
    return 0;
}
