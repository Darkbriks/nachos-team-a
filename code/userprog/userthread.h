#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "thread.h"
#include "system.h"
#include "exception.h"

/**
 * @brief Detach a thread
 *
 * @param thread The thread to detach
 * @return 0 on success, -1 on error (check errno)
 */
extern int do_PthreadDetach(tid_t thread);

void handle_SC_PthreadCreate();
void handle_SC_PthreadExit();
void handle_SC_PthreadJoin();
void handle_SC_PthreadDetach();

void handle_SC_SetTLS();
void handle_SC_GetTLS();

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
