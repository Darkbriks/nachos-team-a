#include "syscall.h"

int counter = 0;
int mutex_id;

int main() {
    // Créer des sémaphores jusqu'à la limite maximale
    int max_semaphores = 512;
    for (int i = 0; i < max_semaphores; i++) {
        if (SemInit(1) < 0) {
            PutString("Erreur création sémaphore, arrêt à l'index: ", 36);
            PutInt(i);
            PutChar('\n');
            Exit(1);
        }
    }

    for (int i = 0; i < max_semaphores; i++) {
        SemDestroy(i);
    }

    return 0;
}