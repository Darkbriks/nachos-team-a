#include "syscall.h"

int main()
{
    char buffer[100];

    GetString(buffer, 100);
    PutString(buffer, 100);

    GetString(buffer, 100);
    PutString(buffer, 100);

    GetString(buffer, 100);
    PutString(buffer, 100);

    PutChar('\n');
}
