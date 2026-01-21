#include "syscall.h"
#include "nos_common.h"
#include "nos_stdio.h"
#include "nos_unistd.h"
#include "nos_errno.h"
#include "nos_string.h"

int connectRequest(int connId, char* cmd_name, char* cmd){
    char response[80];
    int n;
    printf("Sending command %s: %s\n",cmd_name, cmd);
    if (sendto(connId, cmd, strlen(cmd) + 1) < 0) {
        printf("Failed to send %s command, errno: %d\n",cmd_name, errno);
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
        printf("[CLIENT] Server rejected %s: %s\n",cmd_name, response);
        return -1;
    }
    return 0;
}

int openFile(char* filename, int connId){
    int fd = open(filename, 0);
    if (fd < 0) {
        sendto(connId, "ERR 404", 8);
        printf("File can't be send cause it's not found on disk\n");
        return -1;
    }
    return fd;
}

int sendFile(int fd, int connId){
    int sent = 0, n;
    char buffer[CHUNK_SIZE];
    while ((n = read(fd, buffer, CHUNK_SIZE)) > 0) {
        if (sendto(connId, buffer, n) < 0) {
            printf("Failed to send data chunk\n");
            return -1;
        }
        sent += n;
        printf("Sent %d bytes\n", sent);
    }
    sendto(connId, "EOF", 4);
    return sent;
}

int networkPUT(int connId, char* filename){
    int fd, sent;
    printf("Send file : '%s'\n", filename);

    /* Open the file from real filesystem */
    fd = openFile(filename, connId);

    /* Send OK (size unknown, will use EOF marker) */
    char cmd[80];
    unsigned int fileSize;
    fileLen(filename, &fileSize);
    snprintf(cmd, 200, "OK %d", fileSize);
    sendto(connId, cmd, strlen(cmd));

    sent = sendFile(fd, connId);


    printf("File transfer complete: %d bytes sent\n", sent);
    return 0;
}

int networkGET(int connId, char* filename, int size){
    char buffer[CHUNK_SIZE + 1];
    int fd;
    int received = 0;
    int n;

    printf("Try to receive file : '%s' (%d bytes)\n", filename, size);

    /* Create the file in real filesystem */
    fd = open(filename, O_CREATE);

    /* Open for writing */
    if (fd < 0) {
        sendto(connId, "ERR 500", 8);
        printf("Can't create file in disk\n");
        return -1;
    }

    /* Send OK to confirm ready to receive */
    if (sendto(connId, "OK", 3) < 0) {
        printf("Failed to send OK\n");
        close_file(fd);
        return -1;
    }

    /* Receive and write file data */
    while (received < size) {
        n = recvfrom(connId, buffer, CHUNK_SIZE);
        if (n < 0) {
            printf("Failed to receive data, errno: %d\n", errno);
            break;
        }
        if (n == 0) {
            printf("COnnection closed in the middle of the conversation\n");
            break;
        }

        /* Check for EOF marker */
        if (n == 4 && strcmp(buffer, "EOF") == 0) {
            break;
        }

        write(fd,buffer,n);
        received += n;
        printf("Received %d/%d bytes\n", received, size);
    }

    close_file(fd);
    printf("File saved: '%s' (%d bytes)\n", filename, received);

    sendto(connId, "OK", 3);

    return 0;
}
