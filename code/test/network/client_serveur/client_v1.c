#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"

#define SERVER_ADDR 0
#define SERVER_PORT 8080
#define NUM_MESSAGES 5
#define BUFFER_SIZE 64

int main() {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    printf("========================================\n");
    printf("    Client Starting\n");
    printf("    Connecting to server %d:%d\n", SERVER_ADDR, SERVER_PORT);
    printf("========================================\n");

    int connId = connect(SERVER_ADDR, SERVER_PORT, 0);
    if (connId < 0) {
        printf("[Client] ERROR: connect() failed with %d\n", connId);
        return 1;
    }
    printf("[Client] Connected! (connId=%d)\n", connId);

    for (int i = 1; i <= NUM_MESSAGES; i++) {
        strcpy(message, "MSG-");
        char msgNumStr[12];
        itoa(i, msgNumStr, 10);
        strcat(message, msgNumStr);

        printf("[Client] Sending: '%s'\n", message);
        int sent = sendto(connId, message, strlen(message) + 1);

        if (sent < 0) {
            printf("[Client] ERROR: send failed with %d\n", sent);
            break;
        }

        int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("[Client] ERROR: recv failed with %d\n", n);
            break;
        }
        buffer[n] = '\0';
        printf("[Client] Received: '%s'\n", buffer);

        Sleep(100000);
    }

    printf("[Client] Sending disconnect request...\n");
    strcpy(message, "BYE");
    sendto(connId, message, strlen(message) + 1);

    int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[Client] Server response: '%s'\n", buffer);
    }

    close(connId);

    printf("========================================\n");
    printf("    Client Finished Successfully\n");
    printf("========================================\n");

    return 0;
}