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

    VALIDATE_ARG(space->IsValidUserRange(addr, size), E_FAULT);

    char name[MAX_PATH_SIZE];
    VALIDATE_ARG(CopyStringFromUser(addr, name ,MAX_PATH_SIZE), E_FAULT);
    VALIDATE_ARG(fileSystem->Create(name, 0), E_FULL_DISK);

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

    inode_t inode = fileSystem->Open(name);
    VALIDATE_ARG(inode != INVALID_INODE, E_NOENT);

    // 0 is because for the moment the file is at the begin
    const thread_inode_t result = currentThread->AddOpenFile(inode, 0);
    RETURN(result);
}


void handle_SC_Close() {
    thread_inode_t id = reinterpret_cast<thread_inode_t>(machine->ReadRegister(4));
    VALIDATE_ARG(currentThread->IsOpenFile(id), E_BADF);
    inode_t inode = currentThread->GetOpenFile(id, nullptr);
    VALIDATE_ARG(fileSystem->Close(inode), E_FAULT);
    VALIDATE_ARG(currentThread->RemoveOpenFile(id), E_FAULT);
    RETURN(0);
}

#define MIN(a, b) \
    a > b ? b : a

void handle_SC_Write() {
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    thread_inode_t id = reinterpret_cast<thread_inode_t>(machine->ReadRegister(6));

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

    unsigned int seek_pos;
    inode_t inode = currentThread->GetOpenFile(id, &seek_pos);
    fileSystem->Seek(inode, seek_pos);
    while (offset < n) {
        VALIDATE_ARG(CopyFromUserRaw(buffer, addr + offset, MIN(MAX_STRING_SIZE, n - offset)), E_FAULT);
        DEBUG('a', "Write got string: %s\n", buffer);
        if (const int res = fileSystem->Write(inode, buffer, MIN(MAX_STRING_SIZE, n - offset)); res <= 0) { break; }
        else { 
            offset += res; 
            currentThread->setSeek(inode, seek_pos + res);
        }
    }
    RETURN(offset);
}


void handle_SC_Read() {
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    thread_inode_t id = reinterpret_cast<thread_inode_t>(machine->ReadRegister(6));

    GET_PROCESS_ADDRSPACE();
    VALIDATE_ARG(currentThread->IsOpenFile(id), E_BADF);
    VALIDATE_ARG(space->IsValidUserRange(addr, n), E_FAULT);
    VALIDATE_ARG(n >= 0, E_INVAL);
    VALIDATE_ARG(n != 0, 0);
    VALIDATE_ARG(addr <= INT_MAX - n, E_OVERFLOW); // Prevent overflow

    unsigned int seek_pos;
    inode_t inode = currentThread->GetOpenFile(id, &seek_pos);
    fileSystem->Seek(inode, seek_pos);
    if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; }
    {
        char buffer[n];
        int res = fileSystem->Read(inode, buffer, n);
        currentThread->setSeek(inode, seek_pos + res);
        VALIDATE_ARG(CopyToUserRaw(addr, buffer, res), E_FAULT);

        RETURN(res);

    }
}
