#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "system.h"

#define RETURN(value) \
        machine->WriteRegister(2, (int)(value)); \
        return;

#define INT32_MAX 2147483647

#define MAX_PUT_STRING 8192

#endif // EXCEPTION_H
