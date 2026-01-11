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
    if (!CopyStringFromUser(filenameAddr, filename, MAX_PATH_SIZE)) {
        DEBUG('p', "ForkExec: Failed to copy filename from user memory at address 0x%x\n", filenameAddr);
        RETURN(-E_FAULT);
    }
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
    RETURN(newProcess->getPId());
}

void handle_SC_ForkJoin() {
    const int PID_to_wait = machine->ReadRegister(4);
    const int addr_return = machine->ReadRegister(5);
    if (PID_to_wait < 0){
        RETURN(-E_NOSPC);
    }
    if ( (unsigned int) PID_to_wait == currentThread->getProcess()->getPId() ){
        DEBUG('p', "ForkJoin: process %d try to wait himself\n", PID_to_wait); 
        RETURN(-E_INVAL);
    }
    Process* child = Process::FindProcessByPID(PID_to_wait);
    if ( child == nullptr ){
        DEBUG('p', "ForkJoin: process %d try to wait a process who doesn't exist %d\n",currentThread->getProcess()->getPId(), PID_to_wait); 
        RETURN(-E_NOSPC);
    }
    if ( child->getAncestor() != currentThread->getProcess()){
        DEBUG('p', "ForkJoin: process %d try to wait a process who is not his child %d\n",currentThread->getProcess()->getPId(), PID_to_wait); 
        RETURN(-E_NOCPC);
    }

    DEBUG('p', "ForkJoin: process %d wait for process %d\n",currentThread->getProcess()->getPId(), PID_to_wait); 
    currentThread->getProcess()->WaitForChild(child);
    int result = child->getExitCode();

    delete child;

    if (addr_return >= 0) {
        if (machine->WriteMem(addr_return, sizeof(void*), reinterpret_cast<int>(result)) == FALSE) {
            RETURN(-E_INVAL);
        }
    }
    DEBUG('p', "ForkJoin: process %d finish wait for process %d and get exitCode %d\n",currentThread->getProcess()->getPId(), PID_to_wait, result); 
    
    RETURN(0);
}
