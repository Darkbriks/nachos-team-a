/**
 * File Transfer Client for NachOS - Test Program (Real FS)
 */

#include "nos_client.h"
#include "nos_unistd.h"

int main() {
    int connId;
    int receivedSize = 0;
    int sentSize = 0;
    time_t startTime, endTime;
    OpenFileId fd;
    char testData[] = "This is test data being uploaded from client to server. "
                      "NachOS file transfer test for Part IV of Step 6.";
    char verifyBuf[256];
    int n;

    printf("=== NachOS File Transfer Client (Real FS) ===\n");

    /* Create a local test file for upload */
    fd = open("local.txt", O_CREATE);
    if (fd >= 0) {
        write(fd, testData, strlen(testData));
        close_file(fd);
        printf("Created local file: 'local.txt' (%d bytes)\n", strlen(testData));
    }

    /* Connect to server */
    printf("Connecting to server %d:%d...\n", CLIENT_SERVER_ADDR, CLIENT_SERVER_PORT);
    connId = connect(CLIENT_SERVER_ADDR, CLIENT_SERVER_PORT, 0);
    if (connId < 0) {
        printf("Failed to connect, errno: %d\n", errno);
        return -1;
    }
    printf("Connected (connId: %d)\n\n", connId);

    /* Test 1: GET file from server */
    printf("--- Test 1: GET file ---\n");
    if (clientGetFile(connId, "test.txt", "from_s", CLIENT_MAX_FILESIZE,
                      &receivedSize, &startTime, &endTime) == 0) {
        clientDisplayThroughput(receivedSize, startTime, endTime);

        /* Verify downloaded file */
        printf("\nVerifying downloaded file:\n");
        fd = open("from_s", 0);
        if (fd >= 0) {
            n = read(fd, verifyBuf, 255);
            if (n > 0) {
                verifyBuf[n] = '\0';
                printf("Content: %s\n", verifyBuf);
            }
            close_file(fd);
        }
    } else {
        printf("GET failed\n");
    }

    /* Test 2: PUT local file to server */
    printf("\n--- Test 2: PUT file ---\n");
    if (clientPutFile(connId, "local.txt", "upload.txt", CLIENT_MAX_FILESIZE,
                      &startTime, &endTime) == 0) {
        sentSize = strlen(testData);
        clientDisplayThroughput(sentSize, startTime, endTime);
    } else {
        printf("PUT failed\n");
    }

    /* Send QUIT and close */
    printf("\n--- Closing connection ---\n");
    sendto(connId, "QUIT", 5);
    close(connId);

    printf("Client finished\n");
    return 0;
}
