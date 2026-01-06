#include "syscall.h"

int main(){
    ForkExec("./userpages0");
    ForkExec("./userpages1");
    PutChar('h');
    Sleep(100000);
    return 0;
}
