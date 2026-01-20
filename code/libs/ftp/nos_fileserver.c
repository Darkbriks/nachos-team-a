#include "nos_fileserver.h"
#include "nos_unistd.h"
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

/* Handle GET request - send file to client using real filesystem */
int serverHandleGet(int connId, char *filename) {
    char buffer[SERVER_CHUNK_SIZE];
    OpenFileId fd;
    int sent = 0, n;

    printf("[SERVER] GET request for '%s'\n", filename);

    /* Open the file from real filesystem */
    fd = open(filename, 0);
    if (fd < 0) {
        printf("[SERVER] File not found: %s\n", filename);
        sendto(connId, "ERR 404", 8);
        return 0;
    }

    /* Send OK (size unknown, will use EOF marker) */
    sendto(connId, "OK", 3);
    printf("[SERVER] Sending file...\n");

    /* read and send file in chunks */
    while ((n = read(fd, buffer, SERVER_CHUNK_SIZE)) > 0) {
        if (sendto(connId, buffer, n) < 0) {
            printf("[SERVER] Failed to send data chunk\n");
            close_file(fd);
            return -1;
        }
        sent += n;
        printf("[SERVER] Sent %d bytes\n", sent);
    }

    close_file(fd);

    /* Send EOF marker */
    sendto(connId, "EOF", 4);
    printf("[SERVER] File transfer complete: %d bytes sent\n", sent);
    return 0;
}

/* Handle PUT request - receive file from client using real filesystem */
int serverHandlePut(int connId, char *filename, int size) {
    char buffer[SERVER_CHUNK_SIZE + 1];
    OpenFileId fd;
    int received = 0;
    int n;

    printf("[SERVER] PUT request: '%s' (%d bytes)\n", filename, size);

    /* Create the file in real filesystem */
    fd = open(filename, O_CREATE);

    /* Open for writing */
    if (fd < 0) {
        printf("[SERVER] Cannot open file: %s, errno: %d\n", filename, errno);
        sendto(connId, "ERR 500", 8);
        return -1;
    }

    /* Send OK to confirm ready to receive */
    if (sendto(connId, "OK", 3) < 0) {
        printf("[SERVER] Failed to send OK\n");
        close_file(fd);
        return -1;
    }

    /* Receive and write file data */
    while (received < size) {
        n = recvfrom(connId, buffer, SERVER_CHUNK_SIZE);
        if (n < 0) {
            printf("[SERVER] Failed to receive data, errno: %d\n", errno);
            break;
        }
        if (n == 0) {
            printf("[SERVER] Connection closed by client\n");
            break;
        }

        /* Check for EOF marker */
        if (n == 4 && strcmp(buffer, "EOF") == 0) {
            break;
        }

        /* write to file */
        write(fd,buffer,n);
        received += n;
        printf("[SERVER] Received %d/%d bytes\n", received, size);
    }

    close_file(fd);
    printf("[SERVER] File saved: '%s' (%d bytes)\n", filename, received);

    /* Send final confirmation */
    sendto(connId, "OK", 3);

    return 0;
}
