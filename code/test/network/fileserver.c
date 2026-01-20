#include "nos_fileserver.h"
#include "nos_unistd.h"
int main() {
    int listenerId, connId;
    char buffer[128];
    char cmd[16], filename[SERVER_MAX_FILENAME];
    int size;
    int n;
    OpenFileId fd;
    char testData[] = "Hello from NachOS file server! This is a test file for network transfer.";

    printf("=== NachOS File Server (Real FS) ===\n");

    /* Create a test file in the filesystem */
    fd = open("test.txt", O_CREATE);
    if (fd >= 0) {
        write(fd, testData, strlen(testData));
        close_file(fd);
        printf("Created test file: 'test.txt' (%d bytes)\n", strlen(testData));
    }

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
