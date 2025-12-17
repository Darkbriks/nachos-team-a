#include "syscall.h"

void print_error(const char *msg, int msg_len) {
    PutString((char *)msg, msg_len);
    PutString(" (errno=", 8);
    PutInt(GetLastError());
    PutString(")\n", 2);
}

int main() {

    char buffer[64];
    int result;

    PutString("Enter a string: ", 16);
    result = GetString(buffer, -64); // Not a typo, passing negative size to trigger error
    if (result == -1) {
        print_error("\nGetString failed", 17);
    } else {
        PutString("\nYou entered: ", 14);
        PutString(buffer, result);
    }
    PutString("\n", 1);
}