#include "syscall.h"


void super_fun(void * arg){
    PutInt((int) arg);
    PutInt(*(int * ) arg);
    PutString("la vie est belle sans bug\n", 50);
    SemV((sem_t *) arg);
}

int main(){
    sem_t sem;
    SemInit(&sem, 0);
    PutInt(sem);
    PutString("\non est dans le main au début \n", 50);
    CreateThread(super_fun, &sem); 
    PutString("on est dans le main avant attente\n", 50);
    SemP(&sem);
    PutString("on est dans le main après attente\n", 50);
}
