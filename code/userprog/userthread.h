#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "thread.h"
#include "system.h"
#include "exception.h"

/**
 * @brief Create a Nachos user thread
 *
 * @param thread A pointer to store the created thread id
 * @param attr Thread attributes (can be nullptr for default attributes)
 * @param start_routine The function to be executed by the thread
 * @param arg The argument to be passed to the function
 * @param wrapper_addr The address of the thread exit wrapper function
 * @return 0 on success, -1 on error (check errno)
 */
extern int do_PthreadCreate(posix_thread_t *thread, const posix_thread_attr_t *attr, void *(*start_routine)(void *), void *arg, int wrapper_addr);

/**
 * @brief Destruct the caller thread
 *
 * @param retval The return value of the thread
 */
extern void do_PthreadExit(void *retval);

/**
 * @brief Caller will wait other thread to finish
 *
 * @param thread The thread to wait
 * @param thread_return A pointer to store the return value of the joined thread (can be nullptr)
 * @return 0 on success, -1 on error (check errno)
 */
extern int do_PthreadJoin(posix_thread_t thread, void **thread_return);

/**
 * @brief Detach a thread
 *
 * @param thread The thread to detach
 * @return 0 on success, -1 on error (check errno)
 */
extern int do_PthreadDetach(posix_thread_t thread);

void handle_SC_PthreadCreate();
void handle_SC_PthreadExit();
void handle_SC_PthreadJoin();
void handle_SC_PthreadDetach();
void handle_SC_PthreadSelf();

void handle_SC_Pthread_attr_init();
void handle_SC_Pthread_attr_destroy();
void handle_SC_Pthread_attr_setdetachstate();
void handle_SC_Pthread_attr_getdetachstate();

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

#endif //USERTHREAD__H
