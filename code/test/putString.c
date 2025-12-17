#include "syscall.h"

int main() {
    int res = PutString("Hello, World! This is a test of PutString.\n", 43);
	if (res != 43) {
        PutString("Error: PutString did not write the expected number of bytes.\n", 61);
    } else {
        PutString("PutString executed successfully.\n", 33);
    }
	PutString("\n", 1);
}
