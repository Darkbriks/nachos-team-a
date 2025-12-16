#include "syscall.h"

int main()
{
    char buffer[100];

    PutString("Enter your name: ");
    GetString(buffer, 100);

    PutString("Hello, ");
    PutString(buffer);
    PutString("!\n");

    Halt();
}
