#include "userSbrk.h"
#include "system.h"
#include "addrspace.h"
#include "exception.h"
#include "syscall.h"

void handle_SC_Sbrk() {
    int n = machine->ReadRegister(4);

    AddrSpace* space = currentThread->getAddrSpace();
    if (space == nullptr) { RETURN(-E_INVAL); }
    if (n < 0) { RETURN(-E_INVAL); }

    int result = space->Sbrk(n);
    if (result == -1) { RETURN(-E_NOMEM); }

    RETURN(result);
}