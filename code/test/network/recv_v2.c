#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"

int main() {
    int listenerId = listen(8080);
    if (listenerId < 0) {
        printf("Failed to listen on port 8080, errno: %d\n", errno);
        return -1;
    }
    printf("Listening on port 8080...\n");

    int connId = accept(listenerId, -1);
    if (connId < 0) {
        printf("Failed to accept connection, errno: %d\n", errno);
        return -1;
    }
    printf("Accepted connection, connId: %d\n", connId);

    char buffer[100];
    int n = recvfrom(connId, buffer, 100);
    if (n < 0) {
        printf("Failed to receive data, errno: %d\n", errno);
        close(connId);
        close(listenerId);
        return -1;
    } else {
        printf("Received %d bytes: %s\n", n, buffer);
    }

    if (close(connId) < 0) {
        printf("Failed to close connection, errno: %d\n", errno);
        return -1;
    }
    if (close(listenerId) < 0) {
        printf("Failed to close listener, errno: %d\n", errno);
        return -1;
    }

    printf("Connection closed successfully\n");

    return 0;
}