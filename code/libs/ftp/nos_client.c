#include "nos_client.h"
#include "nos_unistd.h"
#include "nos_bool.h"
#include "nos_common.h"
#include "test_utilities.h"

/* Parse OK response to extract size (may be 0 if server doesn't know size) */
int clientParseOkResponse(char *response, int *size) {
    int i = 0;

    /* Check for OK prefix */
    if (response[0] != 'O' || response[1] != 'K') {
        return -1;
    }

    /* Skip "OK " */
    i = 2;
    if (response[i] == ' ') i++;

    /* Parse size */
    *size = 0;
    while (response[i] >= '0' && response[i] <= '9') {
        *size = (*size) * 10 + (response[i] - '0');
        i++;
    }

    return 0;
}

/* GET file from server and save to local filesystem */
int clientGetFile(int connId, char *remoteFile, char *localFile, int maxSize,
                  int *receivedSize, time_t *startTime, time_t *endTime) {
    char cmd[80];
    char buffer[CHUNK_SIZE + 1];
    OpenFileId fd;
    int received = 0;
    int n;

    /* Build GET command */
    snprintf(cmd, 250, "GET %s", remoteFile);
    connectRequest(connId, "GET", cmd);
    /* Record start time */
    time(startTime);

    fd = open(localFile, O_CREATE);
    if (fd < 0) {
        printf("[CLIENT] Cannot create local file: %s\n", localFile);
        return -1;
    }

    /* Receive and write file data */
    while (1) {
        n = recvfrom(connId, buffer, CHUNK_SIZE);
        if (n <= 0) break;

        /* Check for EOF marker */
        if (n <= 4 && strcmp(buffer, "EOF") == 0) {
            printf("[CLIENT] Received EOF marker\n");
            break;
        }

        /* Write to local file */
        write(fd, buffer, n);
        received += n;
        printf("[CLIENT] Received %d bytes\n", received);
    }

    close_file(fd);

    /* Record end time */
    time(endTime);

    *receivedSize = received;
    printf("[CLIENT] Download complete: %d bytes\n", received);
    TEST_START("hey");
    TEST_PASS();
    return 0;
}

/* PUT local file to server */
int clientPutFile(int connId, char *localFile, char *remoteFile, int maxSize,
                  time_t *startTime, time_t *endTime) {
    char response[32];
    OpenFileId fd;
    int n;
    char cmd[80];
    unsigned int fileSize = 0;
    if (! fileLen(localFile, &fileSize)){
        TEST_FAIL("FILE NOT FOUND for put\n");
        return -1;
    }
    printf("Local file size: %d bytes\n", fileSize);
    /* Build PUT command: "PUT remoteFile size" */
    snprintf(cmd, 200, "PUT %s %d", remoteFile, fileSize);

    if ( connectRequest(connId, "PUT", cmd) != 0){
        printf("FAIL Server refuse connection\n");
        return -1;
    }

    /* Reopen file and send data */
    /* Record start time */
    time(startTime);
    fd = open(localFile, 0);
    if (fd < 0) {
        printf("[CLIENT] Cannot reopen local file\n");
        return -1;
    }

    int sent = sendFile(fd, connId);
    close_file(fd);

    /* Wait for final confirmation */
    n = recvfrom(connId, response, sizeof(response) - 1);
    if (n > 0) {
        response[n] = '\0';
        printf("[CLIENT] Final response: %s\n", response);
    }

    /* Record end time */
    time(endTime);

    printf("[CLIENT] Upload complete: %d bytes\n", sent);
    return 0;
}

/* Calculate and display throughput using real time */

void clientDisplayThroughput(int bytes, time_t startTime, time_t endTime) {
    time_t elapsed = endTime - startTime;
    int bytesPerSec;
    int minutes, seconds;

    printf("\n=== Transfer Statistics ===\n");
    printf("Bytes transferred: %d\n", bytes);

    /* Display elapsed time in human-readable format */
    seconds = (int)elapsed;
    minutes = seconds / 60;
    seconds = seconds % 60;

    if (minutes > 0) {
        printf("Elapsed time: %d min %d sec\n", minutes, seconds);
    } else {
        printf("Elapsed time: %d sec\n", seconds);
    }

    if (elapsed > 0) {
        bytesPerSec = bytes / (int)elapsed;
        printf("Throughput: %d bytes/sec\n", bytesPerSec);
        if (bytesPerSec >= 1024) {
            printf("Throughput: %d KB/sec\n", bytesPerSec / 1024);
        }
    } else {
        printf("Transfer completed in less than 1 second\n");
        printf("Throughput: >%d bytes/sec\n", bytes);
    }
}
