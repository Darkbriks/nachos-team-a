#include "syscall.h"
#include "my_stdlib.h"

void get(int n)
{
    int i;
    int ret_val = GetInt(&i);
    if (ret_val == 0){
        PutInt(i);
        PutChar('\n');
        return;
    }

    print_error("Value given was not a number.");

}

int main()
{
    get(4);
}
