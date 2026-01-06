#include "userprocess.h"
#include "system.h"
#include "process.h"
#include "filesys.h"
#include "exception.h"
#include "syscall.h"

static void StartProcess(const int arg) {
    const auto* process = reinterpret_cast<Process*>(arg);

    DEBUG('p', "StartProcess: Starting process %d\n", process->getPId());

    const AddrSpace* space = process->getSpace();
    ASSERT(space != nullptr);

    space->InitRegisters();
    space->RestoreState();

    machine->Run();

    ASSERT(FALSE);
}

void handle_SC_ForkExec() {
    const int filenameAddr = machine->ReadRegister(4);

    if (filenameAddr <= 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)

    char filename[MAX_PATH_SIZE];
    copyStringFromMachine(filenameAddr, filename, MAX_PATH_SIZE);
    DEBUG('p', "ForkExec: Request to execute '%s'\n", filename);

    OpenFile* executable = fileSystem->Open(filename);
    if (executable == nullptr) {
        DEBUG('p', "ForkExec: Unable to open file '%s'\n", filename);
        RETURN(-E_NOENT);
    }

    Process* newProcess = Process::createProcess(executable);
    if (newProcess == nullptr) {
        DEBUG('p', "ForkExec: Failed to create process for '%s'\n", filename);
        delete executable;
        RETURN(-E_NOMEM);
    }

    currentThread->getProcess()->getSpace()->RestoreState();

    Thread* mainThread = newProcess->getMainThread();
    ASSERT(mainThread != nullptr);

    mainThread->Fork(StartProcess, reinterpret_cast<int>(newProcess));

    DEBUG('p', "ForkExec: Successfully launched process %d from '%s'\n", newProcess->getPId(), filename);
    RETURN(0);
}