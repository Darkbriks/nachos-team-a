#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"

int main() {
    int connId = connect(0, 8080, 0);
    if (connId < 0) {
        printf("Failed to connect to server, errno: %d\n", errno);
        return -1;
    }
    printf("Connected to server, connId: %d\n", connId);

    if (sendto(connId, "Hello!", 7) < 0) {
        printf("Failed to send data, errno: %d\n", errno);
        close(connId);
        return -1;
    }
    printf("Data sent successfully\n");

    if (close(connId) < 0) {
        printf("Failed to close connection, errno: %d\n", errno);
        return -1;
    }
    printf("Connection closed successfully\n");

    return 0;
}