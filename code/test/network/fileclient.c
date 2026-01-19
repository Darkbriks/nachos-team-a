/**
 * File Transfer Client for NachOS - Test Program
 */

#include "nos_client.h"

/* Local buffer for received file */
static char localFileBuffer[CLIENT_MAX_FILESIZE];

int main() {
    int connId;
    int receivedSize = 0;
    long long startTime, endTime;
    char testData[] = "This is test data being uploaded from client to server. "
                      "NachOS file transfer test for Part IV of Step 6.";
    int testSize = strlen(testData);

    printf("=== NachOS File Transfer Client ===\n");

    /* Connect to server */
    printf("Connecting to server %d:%d...\n", CLIENT_SERVER_ADDR, CLIENT_SERVER_PORT);
    connId = connect(CLIENT_SERVER_ADDR, CLIENT_SERVER_PORT, 0);
    if (connId < 0) {
        printf("Failed to connect, errno: %d\n", errno);
        return -1;
    }
    printf("Connected (connId: %d)\n\n", connId);

    /* Test 1: GET file */
    printf("--- Test 1: GET file ---\n");
    if (clientGetFile(connId, "test.txt", localFileBuffer, CLIENT_MAX_FILESIZE,
                      &receivedSize, &startTime, &endTime) == 0) {
        localFileBuffer[receivedSize] = '\0';
        printf("\nFile contents:\n%s\n", localFileBuffer);
        clientDisplayThroughput(receivedSize, startTime, endTime);
    } else {
        printf("GET failed\n");
    }

    printf("\n--- Test 2: PUT file ---\n");
    if (clientPutFile(connId, "upload.txt", testData, testSize, &startTime, &endTime) == 0) {
        printf("PUT successful\n");
        clientDisplayThroughput(testSize, startTime, endTime);
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
