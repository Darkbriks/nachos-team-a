// addrspace.h
//      Data structures to keep track of executing user programs
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"

#define UserStackSize 1024 // increase this as necessary!
#define MAX_SEMAPHORES_PER_PROCESS 16 // TODO : Pass this value as a parameter ? If 0, no init of semaphores => no overhead if user don't need semaphores


class BitMapThreadSafe;
class Process;

struct semaphore_descriptor {
    class Semaphore* semaphore;
    bool valid;
};

class AddrSpace {
    public:
        AddrSpace(OpenFile *executable); // Create an address space,
        // initializing it with the program
        // stored in the file "executable"
        ~AddrSpace(); // De-allocate an address space

        void InitRegisters(); // Initialize user-level CPU registers, before jumping to user code

        void SaveState();    // Save/restore address space-specific
        void RestoreState(); // info on a context switch

        unsigned int GetNumPages() const { return numPages; }

        int SemaphoreCreate(int initialValue);
        int SemaphoreWait(int semId);
        int SemaphorePost(int semId);
        int SemaphoreDestroy(int semId);

    private:
        TranslationEntry *pageTable; // Assume linear page table translation for now!
        unsigned int numPages; // Number of pages in the virtual address space

        semaphore_descriptor semaphoreTable[MAX_SEMAPHORES_PER_PROCESS];
        BitMapThreadSafe* semaphoreBitmap;

        class Semaphore* GetSemaphore(int semId);
};

#endif // ADDRSPACE_H
