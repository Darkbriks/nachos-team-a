#include "syscall.h"
#include "nos_stdarg.h"

int sum(int count, ...) {
    va_list args;
    va_start(args, count);
    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }
    va_end(args);
    return total;
}

int main() {
    int result = sum(4, 10, 20, 30, 40);
    PutString("Expected: 100 ; Got: ", 25);
    PutInt(result);
    PutChar('\n');

    if (result == 100) {
        PutString("Test passed!\n", 13);
    } else {
        PutString("Test failed!\n", 13);
    }

    return 0;
}