#include "syscall.h"

int main()
{
    char ch = GetChar();
    PutChar(ch);

    ch = GetChar();
    PutChar(ch);

    ch = GetChar();
    PutChar(ch);

    PutChar('\n');
}
