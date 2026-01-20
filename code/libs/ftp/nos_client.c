#include "nos_client.h"
#include "nos_unistd.h"

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
    char buffer[CLIENT_CHUNK_SIZE + 1];
    OpenFileId fd;
    int received = 0;
    int n;

    /* Build GET command */
    strcpy(cmd, "GET ");
    strcat(cmd, remoteFile);

    /* Record start time */
    time(startTime);

    /* Send GET request */
    printf("[CLIENT] Sending: %s\n", cmd);
    if (sendto(connId, cmd, strlen(cmd) + 1) < 0) {
        printf("[CLIENT] Failed to send GET command\n");
        return -1;
    }

    /* Receive response */
    n = recvfrom(connId, buffer, CLIENT_CHUNK_SIZE);
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

    fd = open(localFile, O_CREATE);
    if (fd < 0) {
        printf("[CLIENT] Cannot create local file: %s\n", localFile);
        return -1;
    }

    /* Receive and write file data */
    while (1) {
        n = recvfrom(connId, buffer, CLIENT_CHUNK_SIZE);
        if (n <= 0) break;

        /* Check for EOF marker */
        if (n == 4 && strcmp(buffer, "EOF") == 0) {
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
    return 0;
}

/* PUT local file to server */
int clientPutFile(int connId, char *localFile, char *remoteFile, int maxSize,
                  time_t *startTime, time_t *endTime) {
    char cmd[80];
    char buffer[CLIENT_CHUNK_SIZE];
    char response[32];
    OpenFileId fd;
    int sent = 0;
    int fileSize = 0;
    int n;

    /* Open local file */
    fd = open(localFile, 0);
    if (fd < 0) {
        printf("[CLIENT] Cannot open local file: %s, fd = %d\n", localFile, fd);
        return -1;
    }

    /* Read file to get size (read and count) */
    {
        char tempBuf[CLIENT_CHUNK_SIZE];
        printf("fd = %d sur name = %s\n", fd, localFile);
        while ((n = read(fd, tempBuf, CLIENT_CHUNK_SIZE)) > 0) {
            printf("hey\n");
            fileSize += n;
            if (fileSize >= maxSize) break;
        }
    }
    close_file(fd);
    printf("[CLIENT] Local file size: %d bytes\n", fileSize);

    /* Build PUT command: "PUT remoteFile size" */
    strcpy(cmd, "PUT ");
    strcat(cmd, remoteFile);
    strcat(cmd, " ");
    intToStr(fileSize, cmd + strlen(cmd));

    /* Record start time */
    time(startTime);

    /* Send PUT request */
    printf("[CLIENT] Sending: %s\n", cmd);
    if (sendto(connId, cmd, strlen(cmd) + 1) < 0) {
        printf("[CLIENT] Failed to send PUT command, errno: %d\n", errno);
        return -1;
    }

    /* Wait for OK */
    n = recvfrom(connId, response, sizeof(response) - 1);
    if (n <= 0) {
        printf("[CLIENT] Failed to receive response\n");
        return -1;
    }
    response[n] = '\0';
    printf("[CLIENT] Response: %s\n", response);

    if (response[0] != 'O') {
        printf("[CLIENT] Server rejected PUT: %s\n", response);
        return -1;
    }

    /* Reopen file and send data */
    fd = open(localFile, 0);
    if (fd < 0) {
        printf("[CLIENT] Cannot reopen local file\n");
        return -1;
    }

    /* Send file data in chunks */
    while ((n = read(fd, buffer, CLIENT_CHUNK_SIZE)) > 0) {
        if (sendto(connId, buffer, n) < 0) {
            printf("[CLIENT] Failed to send data chunk\n");
            close_file(fd);
            return -1;
        }
        sent += n;
        printf("[CLIENT] Sent %d/%d bytes\n", sent, fileSize);
    }

    close_file(fd);

    /* Send EOF */
    sendto(connId, "EOF", 4);

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
