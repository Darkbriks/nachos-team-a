#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"

#define SERVER_ADDR 0
#define SERVER_PORT 8080
#define NUM_MESSAGES 5
#define BUFFER_SIZE 4096

char* msg_p1 = "Le F-1 était un moteur-fusée de très grande puissance, développé par Rocketdyne pour propulser le premier étage (S-IC) du lanceur américain géant Saturn V du programme Apollo. Les cinq moteurs F-1 utilisés sur Saturn V, d'une poussée unitaire au sol de 6,77 méganewtons (690 tonnes) brûlaient de l'oxygène liquide (LOX) et du kérosène (RP-1).";
char* msg_p2 = "\nLe F-1 est toujours en 2025 le moteur-fusée à ergols liquides et à chambre de combustion unique le plus puissant à avoir été mis en service.";
//char* msg_p3 = "\n# Historique\n## La genèse du moteur F-1\n";
//char* msg_p4 = "Les origines du F-1 remontent à 1955, lorsque la compagnie Rocketdyne se voit confier par l'Armée de l'air américaine, la réalisation d'un moteur-fusée à forte puissance, d’une poussée de 4,45 méganewtons. Dès le début, le développement du moteur progresse rapidement. En 1957, Rocketdyne produit les spécifications détaillées de la chambre de combustion et débute les essais sur certains composants du moteur.";

char msg[BUFFER_SIZE];

void prepare_message() {
    strcpy(msg, msg_p1);
    strcat(msg, msg_p2);
    //strcat(msg, msg_p3);
    //strcat(msg, msg_p4);
}

int main() {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    prepare_message();

    printf("========================================\n");
    printf("    Client Starting\n");
    printf("    Connecting to server %d:%d\n", SERVER_ADDR, SERVER_PORT);
    printf("========================================\n");

    int connId = connect(SERVER_ADDR, SERVER_PORT, 0);
    if (connId < 0) {
        printf("[Client] ERROR: connect() failed with %d\n", connId);
        return 1;
    }
    printf("[Client] Connected! (connId=%d)\n", connId);

    for (int i = 1; i <= NUM_MESSAGES; i++) {
        strcpy(message, msg);
        char msgNumStr[12];
        itoa(i, msgNumStr, 10);
        strcat(message, msgNumStr);

        printf("[Client] Sending: '%s'\n", message);
        int sent = sendto(connId, message, strlen(message) + 1);

        if (sent < 0) {
            printf("[Client] ERROR: send failed with %d\n", sent);
            break;
        }

        int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("[Client] ERROR: recv failed with %d\n", n);
            break;
        }
        buffer[n] = '\0';
        printf("[Client] Received: '%s'\n", buffer);

        Sleep(100000);
    }

    printf("[Client] Sending disconnect request...\n");
    strcpy(message, "BYE");
    sendto(connId, message, strlen(message) + 1);

    int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[Client] Server response: '%s'\n", buffer);
    }

    close(connId);

    printf("========================================\n");
    printf("    Client Finished Successfully\n");
    printf("========================================\n");

    return 0;
}