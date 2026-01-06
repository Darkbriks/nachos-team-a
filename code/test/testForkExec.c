#include "syscall.h"

int main(){
    ForkExec("./userpages0");
    ForkExec("./userpages1");
    PutString("All processes launched.\n", 26);
    return 0;
}
