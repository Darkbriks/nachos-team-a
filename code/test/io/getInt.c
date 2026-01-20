#include "nos_errno.h"
#include "syscall.h"
#include "nos_stdio.h"

void get(int n)
{
    int i;
    int ret_val = GetInt(&i);
    if (ret_val == 0){
        PutInt(i);
        PutChar('\n');
        return;
    }

    printf("Value given was not a number. (errno=%d)\n", errno);
}

int main()
{
    get(4);
}
