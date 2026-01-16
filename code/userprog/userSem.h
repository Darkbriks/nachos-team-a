#ifndef USER_SEM_H
#define USER_SEM_H

void handle_SC_SemInit();
void handle_SC_SemWait();
void handle_SC_SemPost();
void handle_SC_SemDestroy();
void handle_SC_SetMaxSemForProcess();

#endif // USER_SEM_H
