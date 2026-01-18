#include "copyright.h"
#include "nos_errno.h"
#include "system.h"
#include "nos_limits.h"
#include "syscall.h"
#include "exception.h"
#include "thread.h"
#include "filesys.h"

#include "userFile.h"

void handle_SC_Create(){
    ptr_32 addr = reinterpret_cast<ptr_32>(machine->ReadRegister(4));

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    // TODO remove 10 for minSize)
    char name[MAX_PATH_SIZE];
    if ( ! CopyStringFromUser(addr, name ,MAX_PATH_SIZE)){
        RETURN(-E_FAULT);
    }
    if (! fileSystem->Create(name, 10)){
        RETURN(-E_FULL_DISK);
    }
    RETURN(0);
}

void handle_SC_Open(){
    ptr_32 addr = reinterpret_cast<ptr_32>(machine->ReadRegister(4));

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    // TODO remove 10 for minSize)
    char name[MAX_PATH_SIZE];
    if ( ! CopyStringFromUser(addr, name ,MAX_PATH_SIZE)){
        RETURN(-E_FAULT);
    }
    if ( ! currentThread->CanOpenFile()){
        RETURN(-E_FTABLE);
    }
    OpenFile* file = fileSystem->Open(name);
    if (file == nullptr){
        RETURN(-E_NOENT);
    }
    RETURN(currentThread->AddOpenFile(file));
}


void handle_SC_Close(){
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(4));
    if ( ! currentThread->IsOpenFile(id)){
        RETURN(-E_BADF);
    }
    OpenFile* file = currentThread->GetOpenFile(id);
    if (! fileSystem->Close(file)){
        RETURN(-E_FAULT);
    }
    RETURN(0);

}

void handle_SC_Write(){
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(6));
    if ( ! currentThread->IsOpenFile(id)){
        RETURN(-E_BADF);
    }

    if (n > MAX_PUT_STRING) {
        n = MAX_PUT_STRING;
    }

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }
    if (n > INT_MAX - addr) { RETURN(-E_OVERFLOW); } // Prevent overflow

    // currentThread->isOpenFIle(fileDescriptor);

    int offset = 0;
    char buffer[MAX_STRING_SIZE];

    OpenFile* file = currentThread->GetOpenFile(id);
    while (offset < n) {
        if (!CopyStringFromUser(addr + offset, buffer, MAX_STRING_SIZE)) {
            RETURN(-E_FAULT);
        }
        DEBUG('a', "Write got string: %s\n", buffer);
        if (const int res = fileSystem->Write(file, buffer, MAX_STRING_SIZE); res <= 0) { break; }
        else { offset += res; }
    }
    RETURN(n);
}


void handle_SC_Read(){
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    OpenFileId id = reinterpret_cast<OpenFileId>(machine->ReadRegister(6));
    if ( ! currentThread->IsOpenFile(id)){
        RETURN(-E_BADF);
    }

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }


    OpenFile* file = currentThread->GetOpenFile(id);
    if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; }
    {
        char buffer[n];
        int res = fileSystem->Read(file, buffer, n);
        if (!CopyStringToUser(buffer, addr, n)) {
            RETURN(-E_FAULT);
        }
        RETURN(res);
    }
}


