#ifndef USER_SEM_H
#define USER_SEM_H
#include "exception.h"

void handle_SC_SemInit();
void handle_SC_SemP();
void handle_SC_SemV();
void handle_SC_SemDestroy();

#endif // USER_SEM_H
