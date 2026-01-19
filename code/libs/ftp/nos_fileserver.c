#include "nos_fileserver.h"

/* File storage - static to this file */
static char fileBuffer[SERVER_MAX_FILESIZE];
static int fileSize = 0;
static char storedFilename[SERVER_MAX_FILENAME];

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

/* Parse command from received data */
int serverParseCommand(char *data, char *cmd, char *arg1, int *arg2) {
    int i = 0, j = 0;

    /* Extract command */
    while (data[i] != '\0' && data[i] != ' ' && j < 15) {
        cmd[j++] = data[i++];
    }
    cmd[j] = '\0';

    /* Skip space */
    if (data[i] == ' ') i++;

    /* Extract first argument (filename) */
    j = 0;
    while (data[i] != '\0' && data[i] != ' ' && j < SERVER_MAX_FILENAME - 1) {
        arg1[j++] = data[i++];
    }
    arg1[j] = '\0';

    /* Extract second argument (size) if present */
    *arg2 = 0;
    if (data[i] == ' ') {
        i++;
        while (data[i] >= '0' && data[i] <= '9') {
            *arg2 = (*arg2) * 10 + (data[i] - '0');
            i++;
        }
    }

    return 0;
}

/* Handle GET request - send file to client */
int serverHandleGet(int connId, char *filename) {
    char response[32];
    int sent = 0;
    int chunkSize;

    printf("[SERVER] GET request for '%s'\n", filename);

    /* Check if we have the file */
    if (strcmp(filename, storedFilename) != 0 || fileSize == 0) {
        printf("[SERVER] File not found: %s\n", filename);
        if (sendto(connId, "ERR 404", 8) < 0) {
            return -1;
        }
        return 0;
    }

    /* Send OK with file size */
    strcpy(response, "OK ");
    intToStr(fileSize, response + 3);
    printf("[SERVER] Sending: %s\n", response);
    if (sendto(connId, response, strlen(response) + 1) < 0) {
        printf("[SERVER] Failed to send OK response\n");
        return -1;
    }

    /* Send file data in chunks */
    while (sent < fileSize) {
        chunkSize = fileSize - sent;
        if (chunkSize > SERVER_CHUNK_SIZE) {
            chunkSize = SERVER_CHUNK_SIZE;
        }

        if (sendto(connId, fileBuffer + sent, chunkSize) < 0) {
            printf("[SERVER] Failed to send data chunk at offset %d\n", sent);
            return -1;
        }

        sent += chunkSize;
        printf("[SERVER] Sent %d/%d bytes\n", sent, fileSize);
    }

    /* Send EOF marker */
    if (sendto(connId, "EOF", 4) < 0) {
        printf("[SERVER] Failed to send EOF\n");
        return -1;
    }

    printf("[SERVER] File transfer complete: %d bytes sent\n", fileSize);
    return 0;
}

/* Handle PUT request - receive file from client */
int serverHandlePut(int connId, char *filename, int size) {
    char buffer[SERVER_CHUNK_SIZE + 1];
    int received = 0;
    int n;

    printf("[SERVER] PUT request: '%s' (%d bytes)\n", filename, size);

    if (size > SERVER_MAX_FILESIZE) {
        printf("[SERVER] File too large\n");
        sendto(connId, "ERR 413", 8);
        return -1;
    }

    /* Store filename */
    strcpy(storedFilename, filename);
    fileSize = size;

    /* Send OK to confirm ready to receive */
    if (sendto(connId, "OK", 3) < 0) {
        printf("[SERVER] Failed to send OK\n");
        return -1;
    }

    /* Receive file data */
    while (received < size) {
        n = recvfrom(connId, buffer, SERVER_CHUNK_SIZE);
        if (n < 0) {
            printf("[SERVER] Failed to receive data, errno: %d\n", errno);
            return -1;
        }
        if (n == 0) {
            printf("[SERVER] Connection closed by client\n");
            break;
        }

        /* Check for EOF marker */
        if (n == 4 && strcmp(buffer, "EOF") == 0) {
            break;
        }

        /* Copy data to buffer */
        memcpy(fileBuffer + received, buffer, n);
        received += n;
        printf("[SERVER] Received %d/%d bytes\n", received, size);
    }

    fileSize = received;
    printf("[SERVER] File stored: '%s' (%d bytes)\n", storedFilename, fileSize);

    /* Send final confirmation */
    sendto(connId, "OK", 3);

    return 0;
}
