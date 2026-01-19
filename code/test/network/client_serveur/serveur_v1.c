#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "pthread.h"

#define SERVER_PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 300

typedef struct {
    int connId;
    int clientNum;
} ClientInfo;

int totalMessages = 0;

void* handle_client(void* arg) {
    ClientInfo* info = (ClientInfo*)arg;
    int connId = info->connId;
    int clientNum = info->clientNum;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int msgCount = 0;

    printf("[Server] Thread started for client #%d (connId=%d)\n", clientNum, connId);

    while (1) {
        int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);

        if (n <= 0) {
            printf("[Server] Client #%d disconnected (n=%d)\n", clientNum, n);
            break;
        }

        buffer[n] = '\0';
        msgCount++;
        totalMessages++;

        printf("[Server] From client #%d: '%s' (msg #%d)\n", clientNum, buffer, msgCount);

        if (strcmp(buffer, "BYE") == 0) {
            strcpy(response, "GOODBYE");
            sendto(connId, response, strlen(response) + 1);
            printf("[Server] Client #%d requested disconnect\n", clientNum);
            break;
        }

        strcpy(response, "ACK-");
        char msgNumStr[12];
        itoa(msgCount, msgNumStr, 10);
        strcat(response, msgNumStr);
        int sent = sendto(connId, response, strlen(response) + 1);

        if (sent < 0) {
            printf("[Server] Send error to client #%d\n", clientNum);
            break;
        }
    }

    close(connId);

    printf("[Server] Thread for client #%d finished. Processed %d messages\n", clientNum, msgCount);

    free(info);
    pthread_exit(0);
    return NULL;
}

int main() {
    printf("========================================\n");
    printf("    Multi-Client Server Starting\n");
    printf("    Port: %d, Max clients: %d\n", SERVER_PORT, MAX_CLIENTS);
    printf("========================================\n");

    int listenerId = listen(SERVER_PORT);
    if (listenerId < 0) {
        printf("[Server] ERROR: listen() failed with %d\n", listenerId);
        return 1;
    }
    printf("[Server] Listening on port %d (listenerId=%d)\n", SERVER_PORT, listenerId);

    pthread_t threads[MAX_CLIENTS];
    int clientCount = 0;

    while (clientCount < MAX_CLIENTS) {
        printf("[Server] Waiting for client #%d...\n", clientCount + 1);

        int connId = accept(listenerId, -1);

        if (connId < 0) {
            printf("[Server] ERROR: accept() failed with %d\n", connId);
            continue;
        }

        clientCount++;
        printf("[Server] Accepted client #%d (connId=%d)\n", clientCount, connId);

        ClientInfo info;
        info.connId = connId;
        info.clientNum = clientCount;

        if (pthread_create(&threads[clientCount - 1], NULL, handle_client, &info) != 0) {
            printf("[Server] ERROR: pthread_create failed for client #%d\n", clientCount);
            close(connId);
            clientCount--;
        }
    }

    printf("[Server] Waiting for all client threads to finish...\n");
    for (int i = 0; i < clientCount; i++) {
        pthread_join(threads[i], NULL);
    }

    close(listenerId);

    printf("========================================\n");
    printf("    Server Shutdown Complete\n");
    printf("    Clients served: %d\n", clientCount);
    printf("    Total messages: %d\n", totalMessages);
    printf("========================================\n");

    return 0;
}