#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"

int main() {
    char* msg = "Hello";
    int to = 1;

    printf("Sending message: %s\n", msg);
    if (sendto(to, msg, 6) < 0) {
        printf("Failed to send message, errno: %d\n", errno);
        return -1;
    }
    printf("Message sent successfully\n");
    return 0;
}