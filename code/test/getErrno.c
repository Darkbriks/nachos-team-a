#include "nos_stdlib.h"

int main() {

    char buffer[64];
    int result;

    PutString("Enter a string: ", 16);
    result = GetString(buffer, -64); // Not a typo, passing negative size to trigger error
    if (result == -1) {
        print_error("\nGetString failed");
    } else {
        PutString("\nYou entered: ", 14);
        PutString(buffer, result);
    }
    PutString("\n", 1);
}
