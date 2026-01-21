/**
 * File Transfer Client for NachOS - Test Program (Real FS)
 */

#include "nos_client.h"
#include "nos_common.h"
#include "nos_unistd.h"
#include "test_utilities.h"

void create_test_file(){
    /* Create a local test file for upload */
    int fd = open("local.txt", O_CREATE);
    if (fd >= 0) {
        char testData[] = "This is test data being uploaded from client to server. "
                          "NachOS file transfer test for Part IV of Step 6.";
        write(fd, testData, strlen(testData));
        close_file(fd);
        printf("Created local file: 'local.txt' (%d bytes)\n", strlen(testData));
    }
}

int connect_to_server(){
    printf("Connecting to server %d:%d...\n", SERVER_ADDR, PORT);
    int result = connect(SERVER_ADDR, PORT, 0);
    if (result < 0) {
        printf("Failed to connect, errno: %d\n", errno);
        TEST_FAIL("Cant open connection\n");
        return -1;
    }
    printf("Connected (connId: %d)\n\n", result);
    return result;
}



void test_GET(int connId){
    int fd, receivedSize = 0;
    int n;
    char verifyBuf[255];
    time_t startTime, endTime;
    printf("--- Test 1: GET file ---\n");
    if (! (clientGetFile(connId, "test.txt", "from_s", MAX_FILESIZE,
                    &receivedSize, &startTime, &endTime) == 0)) {
        TEST_FAIL("Don't receiveFile\n");
    }
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
}

void test_PUT(int connId){
    time_t startTime, endTime;
    printf("\n--- Test 2: PUT file ---\n");
    if ( clientPutFile(connId, "local.txt", "upload.txt", MAX_FILESIZE,
                      &startTime, &endTime) < 0) {
        TEST_FAIL("Don't send the file to the server\n");
    }
    unsigned int sentSize;
    fileLen("local.txt", &sentSize);
    clientDisplayThroughput(sentSize, startTime, endTime);
}

int main() {
    int connId;

    TEST_START("NachOS File Transfer Client (Real FS) ===\n");

    create_test_file();

    connId = connect_to_server();

    test_GET(connId);

    /* Test 2: PUT local file to server */
    test_PUT(connId);
    /* Send QUIT and close */
    printf("\n--- Closing connection ---\n");
    sendto(connId, "QUIT", 5);
    close(connId);
    printf("Client finished\n");
    TEST_PASS();
    return 0;
}
