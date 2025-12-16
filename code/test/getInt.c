#include "syscall.h"

void get(int n)
{
    int i;
    GetInt(&i);
    PutInt(i);
    PutChar('\n');
}

int main()
{
    get(4);
    Halt();
}
