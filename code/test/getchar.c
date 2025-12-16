#include "syscall.h"

int main()
{
    char ch;

    PutString("Type here: ");
    ch = GetChar();

    PutString("You typed: ");
    PutChar(ch);
    PutChar('\n');

    Halt();
}
