#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "system.h"

#define RETURN(value) \
        machine->WriteRegister(2, (int)(value)); \
        return;

#define VALIDATE_ARG(cond, errcode) \
    do { \
        if (!(cond)) { \
            DEBUG('e', "Exception handler: argument validation failed: " #cond "\n"); \
            RETURN(-(errcode)) \
        } \
    } while (0)

#define MAX_PUT_STRING 8192

#endif // EXCEPTION_H
