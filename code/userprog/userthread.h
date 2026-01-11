#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "thread.h"
#include "system.h"
#include "exception.h"

/**
 * @brief Create a Nachos user thread
 *
 * @param thread A pointer to store the created thread id
 * @param start_routine The function to be executed by the thread
 * @param arg The argument to be passed to the function
 * @param wrapper_addr The address of the thread exit wrapper function
 * @return 0 on success, -1 on error (check errno)
 */
extern int do_PthreadCreate(tid_t *thread, void *(*start_routine)(void *), void *arg, int wrapper_addr);

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
void handle_SC_thread_join();
void handle_SC_thread_self();
void handle_SC_thread_yield();

class Param{
    private:
        ptr_32 function ;
        ptr_32 arg;
        ptr_32 exit_addr;
    public:
        ptr_32 get_function() const{return function;}
        ptr_32 get_arg() const{return arg;}
        ptr_32 get_exit_addr() const{return exit_addr;}
        Param(const ptr_32 f, const ptr_32 a, const ptr_32 exit = 0){function=f; arg=a; exit_addr=exit;}
};

struct ThreadStartParams {
    int entryPoint;
    int arg;
    unsigned int stackPtr;
    unsigned int tlsPtr;
    bool kernelAllocatedStack;
    unsigned int stackBase;
};

#endif //USERTHREAD__H
