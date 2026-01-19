#include "nos_client.h"

/* Convert int to string (simple itoa) - static to avoid conflicts */
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

/* GET file from server */
int clientGetFile(int connId, char *filename, char *fileBuffer, int bufSize,
                  int *receivedSize, long long *startTime, long long *endTime) {
    char cmd[80];
    char buffer[CLIENT_CHUNK_SIZE + 1];
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
    if (clientParseOkResponse(buffer, &expectedSize) < 0) {
        printf("[CLIENT] Invalid response format\n");
        return -1;
    }
    printf("[CLIENT] Expecting %d bytes\n", expectedSize);

    /* Receive file data */
    while (received < expectedSize) {
        n = recvfrom(connId, buffer, CLIENT_CHUNK_SIZE);
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
        if (received + n <= bufSize) {
            memcpy(fileBuffer + received, buffer, n);
        }
        received += n;
        printf("[CLIENT] Received %d/%d bytes\n", received, expectedSize);
    }

    /* Wait for EOF if not received yet */
    if (received >= expectedSize) {
        n = recvfrom(connId, buffer, CLIENT_CHUNK_SIZE);
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
int clientPutFile(int connId, char *filename, char *data, int size,
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
        if (chunkSize > CLIENT_CHUNK_SIZE) {
            chunkSize = CLIENT_CHUNK_SIZE;
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
void clientDisplayThroughput(int bytes, long long startTime, long long endTime) {
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
