#include "syscall.h"

int main() {
    int res = PutString("Heeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee789llo\n", 260);
    if (res != 260) {
        PutString("Error: PutString did not write the expected number of bytes.\n", 61);
    } else {
        PutString("PutString executed successfully.\n", 33);
    }
    PutString("\n", 1);
}
