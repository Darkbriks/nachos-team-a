#include "syscall.h"

int main()
{
    char buffer[100];

    PutString("Enter your name: ", 17);
    int res = GetString(buffer, 100);

    PutString("Hello, ", 7);
    PutString(buffer, 100);
    PutString("\nYou typed: ", 11); PutInt(res); PutString(" characters.\n", 13);
    PutString("\n", 1);
}
