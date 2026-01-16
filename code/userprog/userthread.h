#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "exception.h"


void handle_SC_thread_create();
void handle_SC_thread_exit();
void handle_SC_thread_self();
void handle_SC_thread_yield();

struct ThreadStartParams {
    int entryPoint;
    int arg;
    ThreadStartParams(int e, int a){entryPoint = e; arg = a;} 
};

#endif //USERTHREAD__H
