#include "nos_errno.h"
#include "syscall.h"

void get(int n)
{
    int i;
    int ret_val = GetInt(&i);
    if (ret_val == 0){
        PutInt(i);
        PutChar('\n');
    }
}

int main()
{
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
    get(4);
}
