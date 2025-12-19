#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "thread.h"
#include "system.h"
#include "exception.h"

/**
 * @brief Create a thread
 *
 * @param function  the funtion to executes
 * @param arg  a pointer for the argument of the function
 * @param exit_addr  address of ExitThread function (for automatic termination)
 * @return the TID of the new thread. Return -1 if something bad happens
 */
extern int do_UserThreadCreate(int function, int arg, int exit_addr = 0);


void handle_SC_CreateThread();
void handle_SC_JoinThread();

/**
 * @brief Destruct the caller thread 
 */
void do_UserThreadExit();

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
