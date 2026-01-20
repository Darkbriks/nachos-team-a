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
char* msg_p3 = "\n# Historique\n## La genèse du moteur F-1";
char* msg_p4 = "\nLes origines du F-1 remontent à 1955, lorsque la compagnie Rocketdyne se voit confier par l'Armée de l'air américaine, la réalisation d'un moteur-fusée à forte puissance, d’une poussée de 4,45 méganewtons. Dès le début, le développement du moteur progresse rapidement. En 1957, Rocketdyne produit les spécifications détaillées de la chambre de combustion et débute les essais sur certains composants du moteur.";
char* msg_p5 = "\nEn 1958, le projet est abandonné par l'Armée de l'air, qui considère qu'un moteur de ce type est trop puissant pour ses besoins. La NASA, qui dès sa création suit de près le développement du F-1, décide d’émettre un appel d'offres le 14 octobre 1958 pour un moteur développant une poussée de 6,7 méganewtons. Le 9 décembre 1958, la NASA annonce avoir sélectionné Rocketdyne. Elle signe un contrat en ce sens le 9 janvier 1959[1].";
char* msg_p6 = "\nDurant la phase de conception, entre 1959 et 1961, le moteur est simplifié[2] :";
char* msg_p7 = "\n- Abandon du générateur de gaz à monergol utilisant de l'hydrazine stocké dans un réservoir sur le pas de tir au profit d'un générateur de gaz utilisant les ergols du moteur-fusée.";
char* msg_p8 = "\n- Abandon des 3 turbopompes (hydrazine, RP-1 et LOX) au profit d'une seule turbopompe commune au RP-1 et LOX.";
char* msg_p9 = "\nAprès les premiers tests statiques, la simplification va se poursuivre :";
char* msg_p10 = "\n- Simplification de l'injection de RP-1 depuis une triple rampe d'injection vers une seule rampe permettant de supprimer quatre vannes d'admission entre ces trois rampes.";
char* msg_p11 = "\n- Suppression de la régulation active de la puissance provoquant un fonctionnement instable du moteur-fusée.";
char* msg_p12 = "\nLes moteurs sont construits à Canoga Park en Californie. L'intégralité des tests réalisés par Rocketdyne se déroulent sur le site de Edwards Air Force Base en Californie. Le site d'essai est situé à moins de 150 km du lieu de fabrication.";
char* msg_p13 = "\nLe premier test statique du F-1 va avoir lieu le 25 mai 1961, jour du discours Special Message to the Congress on Urgent National Needs du président des États-Unis, John F. Kennedy, annonçant sa volonté d'envoyer un Américain poser le pied sur la Lune avant la fin des années 1960[2].";

char msg[BUFFER_SIZE];

void prepare_message() {
    strcpy(msg, msg_p1);
    strcat(msg, msg_p2);
    strcat(msg, msg_p3);
    strcat(msg, msg_p4);
    strcat(msg, msg_p5);
    strcat(msg, msg_p6);
    strcat(msg, msg_p7);
    strcat(msg, msg_p8);
    strcat(msg, msg_p9);
    strcat(msg, msg_p10);
    strcat(msg, msg_p11);
    strcat(msg, msg_p12);
    strcat(msg, msg_p13);
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
        snprintf(message, BUFFER_SIZE, "--- Message %d ---\n%s\n", i, msg);

        printf("[Client] Sending: '%s' (%d)\n", message, strlen(message));
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