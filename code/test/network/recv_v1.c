#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"

int main() {
    char buffer[100];
    int from = 0;

    printf("Waiting to receive message...\n");
    if (recvfrom(from, buffer, sizeof(buffer)) < 0) {
        printf("Failed to receive message, errno: %d\n", errno);
        return -1;
    }
    printf("Received message: %s\n", buffer);

    return 0;
}