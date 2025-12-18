#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

pthread_cond_t cond;
pthread_mutex_t mut;

void *fun(void *arg){
    pthread_cond_wait(&cond, &mut);
    return NULL;
}

int main(){
    pthread_mutex_init(&mut, NULL);
    pthread_cond_init(&cond, NULL);
    unsigned long long number_thread = 0;
    pthread_t thread;
    while ( pthread_create(&thread, NULL, fun, NULL) == 0){
        number_thread++;
    }
    printf("Fail %d\n", errno );
    printf("EAGAIN = 11 veut dir eplus possbile de créer de thread pour ce processus");
    printf("max_thread = %lld\n", number_thread);
    pthread_cond_signal(&cond);
    printf("Peut on en créer un nouveau ?\n");
    while ( pthread_create(&thread, NULL, fun, NULL) != 0){}
    printf("on a pu en créer un nouveau après la terminaison d'un autre\n");


}

/*
 * man pthread_create 
  VALEUR RENVOYÉE
 En cas de réussite, pthread_create() renvoie 0 ; en cas d'erreur, elle renvoie un numéro d'erreur, et le contenu de *thread est indéfini.

 ERREURS
 EAGAIN Ressources insuffisantes pour créer un nouveau processus léger.

 EAGAIN A system-imposed limit on the number of threads was encountered. There are a number of limits that may trigger this error: the RLIMIT_NPROC soft resource limit (set
 via setrlimit(2)), which limits the number of processes and threads for a real user ID, was reached; the kernel's system-wide limit on the number of  processes  and
 threads, /proc/sys/kernel/threads-max, was reached (see proc(5)); or the maximum number of PIDs, /proc/sys/kernel/pid_max, was reached (see proc(5)).

 *
 *
 *
 *
 */
