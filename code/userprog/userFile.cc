#include "copyright.h"
#include "nos_errno.h"
#include "system.h"
#include "nos_limits.h"
#include "syscall.h"
#include "exception.h"
#include "thread.h"
#include "filesys.h"
#include "process.h"

#include "userFile.h"

void handle_SC_Create() {
    ptr_32 addr = reinterpret_cast<ptr_32>(machine->ReadRegister(4));
    int size = machine->ReadRegister(5);

    GET_PROCESS_ADDRSPACE();

    VALIDATE_ARG(space->IsValidUserRange(addr, MAX_PATH_SIZE), E_FAULT);

    char name[MAX_PATH_SIZE];
    VALIDATE_ARG(CopyStringFromUser(addr, name ,MAX_PATH_SIZE), E_FAULT);
    VALIDATE_ARG(fileSystem->Create(name, size), E_FULL_DISK);

    RETURN(0);
}

void handle_SC_Open() {
    ptr_32 addr = reinterpret_cast<ptr_32>(machine->ReadRegister(4));
    int size = machine->ReadRegister(5);

    GET_PROCESS_ADDRSPACE();
    VALIDATE_ARG(space->IsValidUserRange(addr, size), E_FAULT);

    char name[MAX_PATH_SIZE];
    VALIDATE_ARG(CopyStringFromUser(addr, name ,MAX_PATH_SIZE), E_FAULT);
    VALIDATE_ARG(currentThread->CanOpenFile() != INVALID_ID, E_FTABLE);

    OpenFile* file = fileSystem->Open(name);
    VALIDATE_ARG(file != nullptr, E_NOENT);

    const OpenFileId result = currentThread->AddOpenFile(file);
    RETURN(result);
}


void handle_SC_Close() {
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(4));
    VALIDATE_ARG(currentThread->IsOpenFile(id), E_BADF);
    OpenFile* file = currentThread->GetOpenFile(id);
#ifdef FILESYS
    VALIDATE_ARG(fileSystem->Close(file), E_FAULT);
#else
    delete file;
#endif
    VALIDATE_ARG(currentThread->RemoveOpenFile(id), E_FAULT);
    RETURN(0);
}

void handle_SC_Write() {
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(6));

    GET_PROCESS_ADDRSPACE();

    VALIDATE_ARG(currentThread->IsOpenFile(id), E_BADF);

    if (n > MAX_PUT_STRING) { n = MAX_PUT_STRING; }

    VALIDATE_ARG(space->IsValidUserRange(addr, n), E_FAULT);
    VALIDATE_ARG(n >= 0, E_INVAL);
    VALIDATE_ARG(n != 0, 0);
    VALIDATE_ARG(addr <= INT_MAX - n, E_OVERFLOW); // Prevent overflow

    // currentThread->isOpenFIle(fileDescriptor);

    int offset = 0;
    char buffer[MAX_STRING_SIZE];

    OpenFile* file = currentThread->GetOpenFile(id);
    while (offset < n) {
        int toWrite = n - offset;
        if (toWrite > MAX_STRING_SIZE) { toWrite = MAX_STRING_SIZE; }
        VALIDATE_ARG(CopyFromUserRaw(buffer, addr + offset, toWrite), E_FAULT);
        DEBUG('a', "Write %d bytes to file\n", toWrite);
        int res = file->Write(buffer, toWrite);
        if (res <= 0) { break; }
        offset += res;
    }
    RETURN(offset);
}


void handle_SC_Read() {
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(6));

    GET_PROCESS_ADDRSPACE();
    VALIDATE_ARG(currentThread->IsOpenFile(id), E_BADF);
    VALIDATE_ARG(space->IsValidUserRange(addr, n), E_FAULT);
    VALIDATE_ARG(n >= 0, E_INVAL);
    VALIDATE_ARG(n != 0, 0);
    VALIDATE_ARG(addr <= INT_MAX - n, E_OVERFLOW); // Prevent overflow

    OpenFile* file = currentThread->GetOpenFile(id);
    if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; }
    {
        char buffer[n];
        int res = file->Read(buffer, n);
        if (res > 0) {
            VALIDATE_ARG(CopyToUserRaw(addr, buffer, res), E_FAULT);
        }
        RETURN(res);
    }
}
