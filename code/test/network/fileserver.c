#include "nos_fileserver.h"

static char localFileBuffer[SERVER_MAX_FILESIZE];
static int localFileSize = 0;
static char localStoredFilename[SERVER_MAX_FILENAME];

int main() {
    int listenerId, connId;
    char buffer[128];
    char cmd[16], filename[SERVER_MAX_FILENAME];
    int size;
    int n;

    /* Initialize stored file with some test data */
    strcpy(localStoredFilename, "test.txt");
    strcpy(localFileBuffer, "Hello from NachOS file server! This is a test file for network transfer.");
    localFileSize = strlen(localFileBuffer);

    printf("=== NachOS File Server ===\n");
    printf("Pre-loaded file: '%s' (%d bytes)\n", localStoredFilename, localFileSize);

    /* Start listening */
    listenerId = listen(SERVER_PORT);
    if (listenerId < 0) {
        printf("Failed to listen on port %d, errno: %d\n", SERVER_PORT, errno);
        return -1;
    }
    printf("Listening on port %d...\n", SERVER_PORT);

    /* Accept one connection (simple server) */
    connId = accept(listenerId, -1);
    if (connId < 0) {
        printf("Failed to accept connection, errno: %d\n", errno);
        close(listenerId);
        return -1;
    }
    printf("Client connected (connId: %d)\n", connId);

    /* Main server loop - handle commands */
    while (1) {
        /* Receive command */
        n = recvfrom(connId, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            printf("Client disconnected\n");
            break;
        }
        buffer[n] = '\0';
        printf("[SERVER] Received command: %s\n", buffer);

        /* Parse and handle command */
        serverParseCommand(buffer, cmd, filename, &size);

        if (strcmp(cmd, "GET") == 0) {
            if (serverHandleGet(connId, filename) < 0) {
                break;
            }
        } else if (strcmp(cmd, "PUT") == 0) {
            if (serverHandlePut(connId, filename, size) < 0) {
                break;
            }
        } else if (strcmp(cmd, "QUIT") == 0) {
            printf("[SERVER] Client requested quit\n");
            break;
        } else if (strcmp(cmd, "EOF") == 0) {
            printf("[SERVER] Reception complete!\n");
            break;
        } else {
            printf("[SERVER] Unknown command: %s\n", cmd);
            sendto(connId, "ERR 400", 8);
        }
    }

    /* Cleanup */
    close(connId);
    close(listenerId);
    printf("Server shutdown\n");

    return 0;
}
