#include "syscall.h"

int main()
{
    char buffer[100];

    PutString("Enter your name: ", 17);
    GetString(buffer, 100);

    PutString("Hello, ", 7);
    PutString(buffer, 100);

    Halt();
}
