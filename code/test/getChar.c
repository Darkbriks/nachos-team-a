#include "syscall.h"

int main()
{
    char ch;

    PutString("Type here: ", 11);
    ch = GetChar();

    PutString("You typed: ", 11);
    PutChar(ch);
    PutChar('\n');
}
