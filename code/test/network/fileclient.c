/**
 * File Transfer Client for NachOS
 *
 * This client connects to a file server and performs
 * file transfer operations (GET/PUT).
 *
 * Usage: Performs a GET operation to download a test file,
 *        measures transfer time, and calculates throughput.
 */

#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"
#include "nos_string.h"

#define SERVER_ADDR 0
#define SERVER_PORT 8080
#define CHUNK_SIZE 20
#define MAX_FILESIZE 4096

static char fileBuffer[MAX_FILESIZE];

/* Convert int to string (simple itoa) */
static int intToStr(int num, char *buf) {
    int i = 0, j;
    char tmp[16];

    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    while (num > 0) {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }

    /* Reverse */
    for (j = 0; j < i; j++) {
        buf[j] = tmp[i - 1 - j];
    }
    buf[i] = '\0';
    return i;
}

/* Parse OK response to extract size */
static int parseOkResponse(char *response, int *size) {
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

/* GET file from server */
static int getFile(int connId, char *filename, int *receivedSize,
                   long long *startTime, long long *endTime) {
    char cmd[80];
    char buffer[CHUNK_SIZE + 1];
    int expectedSize = 0;
    int received = 0;
    int n;

    /* Build GET command */
    strcpy(cmd, "GET ");
    strcat(cmd, filename);

    /* Record start time */
    GetCurrentTick(startTime);

    /* Send GET request */
    printf("[CLIENT] Sending: %s\n", cmd);
    if (sendto(connId, cmd, strlen(cmd) + 1) < 0) {
        printf("[CLIENT] Failed to send GET command\n");
        return -1;
    }

    /* Receive response */
    n = recvfrom(connId, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        printf("[CLIENT] Failed to receive response\n");
        return -1;
    }
    buffer[n] = '\0';
    printf("[CLIENT] Response: %s\n", buffer);

    /* Check for error */
    if (buffer[0] == 'E') {
        printf("[CLIENT] Server error: %s\n", buffer);
        return -1;
    }

    /* Parse OK response */
    if (parseOkResponse(buffer, &expectedSize) < 0) {
        printf("[CLIENT] Invalid response format\n");
        return -1;
    }
    printf("[CLIENT] Expecting %d bytes\n", expectedSize);

    /* Receive file data */
    while (received < expectedSize) {
        n = recvfrom(connId, buffer, CHUNK_SIZE);
        if (n < 0) {
            printf("[CLIENT] Failed to receive data, errno: %d\n", errno);
            return -1;
        }
        if (n == 0) {
            printf("[CLIENT] Connection closed\n");
            break;
        }

        /* Check for EOF marker */
        if (n == 4 && strcmp(buffer, "EOF") == 0) {
            printf("[CLIENT] Received EOF marker\n");
            break;
        }

        /* Store received data */
        if (received + n <= MAX_FILESIZE) {
            memcpy(fileBuffer + received, buffer, n);
        }
        received += n;
        printf("[CLIENT] Received %d/%d bytes\n", received, expectedSize);
    }

    /* Wait for EOF if not received yet */
    if (received >= expectedSize) {
        n = recvfrom(connId, buffer, CHUNK_SIZE);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[CLIENT] Final message: %s\n", buffer);
        }
    }

    /* Record end time */
    GetCurrentTick(endTime);

    *receivedSize = received;
    return 0;
}

/* PUT file to server */
static int putFile(int connId, char *filename, char *data, int size,
                   long long *startTime, long long *endTime) {
    char cmd[80];
    char buffer[32];
    int sent = 0;
    int chunkSize;
    int n;

    /* Build PUT command: "PUT filename size" */
    strcpy(cmd, "PUT ");
    strcat(cmd, filename);
    strcat(cmd, " ");
    intToStr(size, cmd + strlen(cmd));

    /* Record start time */
    GetCurrentTick(startTime);

    /* Send PUT request */
    printf("[CLIENT] Sending: %s\n", cmd);
    if (sendto(connId, cmd, strlen(cmd) + 1) < 0) {
        printf("[CLIENT] Failed to send PUT command\n");
        return -1;
    }

    /* Wait for OK */
    n = recvfrom(connId, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        printf("[CLIENT] Failed to receive response\n");
        return -1;
    }
    buffer[n] = '\0';
    printf("[CLIENT] Response: %s\n", buffer);

    if (buffer[0] != 'O') {
        printf("[CLIENT] Server rejected PUT: %s\n", buffer);
        return -1;
    }

    /* Send file data in chunks */
    while (sent < size) {
        chunkSize = size - sent;
        if (chunkSize > CHUNK_SIZE) {
            chunkSize = CHUNK_SIZE;
        }

        if (sendto(connId, data + sent, chunkSize) < 0) {
            printf("[CLIENT] Failed to send data chunk\n");
            return -1;
        }

        sent += chunkSize;
        printf("[CLIENT] Sent %d/%d bytes\n", sent, size);
    }

    /* Send EOF */
    if (sendto(connId, "EOF", 4) < 0) {
        printf("[CLIENT] Failed to send EOF\n");
        return -1;
    }

    /* Wait for final confirmation */
    n = recvfrom(connId, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[CLIENT] Final response: %s\n", buffer);
    }

    /* Record end time */
    GetCurrentTick(endTime);

    return 0;
}

/* Calculate and display throughput */
static void displayThroughput(int bytes, long long startTime, long long endTime) {
    int elapsed = (int)(endTime - startTime);
    int bytesPerSec;

    printf("\n=== Transfer Statistics ===\n");
    printf("Bytes transferred: %d\n", bytes);
    printf("Start tick: %d\n", (int)startTime);
    printf("End tick: %d\n", (int)endTime);
    printf("Elapsed ticks: %d\n", elapsed);

    /*
     * NachOS timing model:
     * - SystemTick = 10 (context switch every 10 ticks)
     * - TimerTicks = 100 (timer interrupt every 100 ticks)
     * - Approximately 100,000 ticks = 1 second of simulated time
     *
     * Throughput calculation:
     * bytes_per_second = bytes * 100000 / elapsed_ticks
     * KB_per_second = bytes_per_second / 1024
     */
    if (elapsed > 0) {
        /* Calculate bytes per second: bytes * 100000 / elapsed */
        /* To avoid overflow: (bytes * 100) / (elapsed / 1000) when elapsed > 1000 */
        if (elapsed > 1000) {
            bytesPerSec = (bytes * 100) / (elapsed / 1000);
        } else {
            bytesPerSec = (bytes * 100000) / elapsed;
        }
        printf("Throughput: %d bytes/second\n", bytesPerSec);
        printf("Throughput: %d KB/s\n", bytesPerSec / 1024);
    } else {
        printf("Transfer too fast to measure\n");
    }
}

int main() {
    int connId;
    int receivedSize = 0;
    long long startTime, endTime;
    char testData[] = "This is test data being uploaded from client to server. "
                      "NachOS file transfer test for Part IV of Step 6.";
    int testSize = strlen(testData);

    printf("=== NachOS File Transfer Client ===\n");

    /* Connect to server */
    printf("Connecting to server %d:%d...\n", SERVER_ADDR, SERVER_PORT);
    connId = connect(SERVER_ADDR, SERVER_PORT, 0);
    if (connId < 0) {
        printf("Failed to connect, errno: %d\n", errno);
        return -1;
    }
    printf("Connected (connId: %d)\n\n", connId);

    /* Test 1: GET file */
    printf("--- Test 1: GET file ---\n");
    if (getFile(connId, "test.txt", &receivedSize, &startTime, &endTime) == 0) {
        fileBuffer[receivedSize] = '\0';
        printf("\nFile contents:\n%s\n", fileBuffer);
        displayThroughput(receivedSize, startTime, endTime);
    } else {
        printf("GET failed\n");
    }

    printf("\n--- Test 2: PUT file ---\n");
    if (putFile(connId, "upload.txt", testData, testSize, &startTime, &endTime) == 0) {
        printf("PUT successful\n");
        displayThroughput(testSize, startTime, endTime);
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
