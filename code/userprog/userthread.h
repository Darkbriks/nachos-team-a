#ifndef USERTHREAD__H
#define USERTHREAD__H

#include "thread.h"
#include "system.h"

/**
 * @brief Create a thread
 *
 * @param function  the funtion to executes
 * @param arg  a pointer for the argument of the function
 * @return the TID of the new thread. Return -1 if something bad happens
 */
extern int do_UserThreadCreate(int function, int arg);

/**
 * @brief Destruct the caller thread 
 * TODO make it better for space. Only the last thead in the process need to free 
 */
void do_UserThreadExit();


class Param{
    private:
        ptr_32 function ;
        ptr_32 arg;
    public:
        ptr_32 get_function() const{return function;}
        ptr_32 get_arg() const{return arg;}
        Param(const ptr_32 f, const ptr_32 a){function=f; arg =a;}
};

#endif //USERTHREAD__H
