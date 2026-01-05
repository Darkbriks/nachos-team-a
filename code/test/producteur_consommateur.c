#include "types.h"
#include "nos_stdlib.h"
#include "syscall.h"

#define SIZE_LIST 71

typedef struct {
    sem_t resource;
    sem_t empty_list;
    int nbProd;
} producteurs_consommateur_t;

typedef struct {
    struct linked_list_head *list;
    int iterations;
    int donnee[SIZE_LIST];
    int nb_data_dispo;
    producteurs_consommateur_t producteurs_consommateur;
} donnees_thread_t;

void consommateur(void *args) {
    donnees_thread_t *d = args;
    int i, valeur;
    for (i=0; i < d->iterations; i++) {
        if (SemWait(d->producteurs_consommateur.empty_list) != 0){
            print_error("bas semaphore");
        }

        if ( SemWait(d->producteurs_consommateur.resource) != 0){
            print_error("bas semaphore");
        }
        valeur = d->donnee[d->nb_data_dispo];
        d->nb_data_dispo--;
        if (d->nb_data_dispo < -1){
            printf_simple("ERREUR");
        }
        if (valeur != d->donnee[d->nb_data_dispo + 1]){
            printf_simple("LECTURE INCOHERENTE !!!\n");
            PutInt(valeur);
            printf_simple("\n");
            PutInt(d->donnee[d->nb_data_dispo + 1]);
            Halt();
        }
        SemPost(d->producteurs_consommateur.resource);
    }
}

void producteurs(void *args) {
    donnees_thread_t *d = args;
    int i, valeur;
    
    for (i=0; i < d->iterations; i++) {

        SemWait(d->producteurs_consommateur.resource);
        valeur = PthreadSelf() * 30 + i;

        d->nb_data_dispo++;
        d->donnee[d->nb_data_dispo] = valeur;
        if (valeur != d->donnee[d->nb_data_dispo]){
            printf_simple("REDACTION INCOHERENTE !!!\n");
            Halt();
        }
        SemPost(d->producteurs_consommateur.resource);
        SemPost(d->producteurs_consommateur.empty_list);
    }
}



int main(int argc, char *argv[]) {
    posix_thread_t threads[100];
    donnees_thread_t donnees_thread;
    int i, nb_consommateurs, nb_producteurs;
    void *resultat;
    
    nb_consommateurs = 10;
    nb_producteurs = 10;
    donnees_thread.iterations = 10;
    donnees_thread.nb_data_dispo = -1;
    
    // -- TESTS --
    
    donnees_thread.producteurs_consommateur.resource = SemInit(1);
    donnees_thread.producteurs_consommateur.empty_list = SemInit(0);
    donnees_thread.producteurs_consommateur.nbProd = 0;
    unsigned int index = 0;

    for (i=0; i<nb_producteurs; i++){
        PthreadCreate(&threads[index ++], NULL, (void *) (void *)producteurs, &donnees_thread);
    } 
    for (i=0; i<nb_consommateurs; i++){
        if ( PthreadCreate(&threads[index ++], NULL, (void *)(void *)consommateur, &donnees_thread) != 0){
            print_error("ça a pas crée le thread\n");
        }
    }


    for (i=0; i<nb_consommateurs+nb_producteurs; i++)
        PthreadJoin(threads[i], &resultat);

    if (donnees_thread.nb_data_dispo != -1){
        printf_simple("ERREUR Ca casse !\n");
        PutInt(donnees_thread.nb_data_dispo);
    }
    SemDestroy(donnees_thread.producteurs_consommateur.resource);
    SemDestroy(donnees_thread.producteurs_consommateur.empty_list);
    return 0;
}

