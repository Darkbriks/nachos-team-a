#include "syscall.h"

void print(int n)
{
    int i;
    for (i = 0; i < n; i++) {
        PutInt(i);
    }
    PutChar('\n');
    PutInt(-1);
    PutChar('\n');
    PutInt(9999999); // Don't worry it's the good value
    PutChar('\n');
}

int main()
{
    print(4);
    Halt();
}
