#include "syscall.h"

int main(){
    for (int i = 0; i < 12; i++){
        ForkExec("./userpages0");
        ForkExec("./userpages1");
    }
    PutString("All processes launched.\n", 26);
    return 0;
}
