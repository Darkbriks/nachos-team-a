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

#define UserStackSize 16384 // increase this as necessary!
#define INITIAL_SEMAPHORE_TABLE_SIZE 16
#define MAX_SEMAPHORES_PER_PROCESS 512 // Arbitrary limit, can be adjusted as needed

#define INITIAL_HEAP_PAGES 2
#define MAX_HEAP_PAGES  256

class BitMapThreadSafe;
class Process;

struct semaphore_descriptor {
    class Semaphore* semaphore;
    bool valid;
};

class AddrSpace {
    public:
        explicit AddrSpace(OpenFile *executable); // Create an address space,
        // initializing it with the program
        // stored in the file "executable"
        ~AddrSpace(); // De-allocate an address space

        void InitRegisters() const; // Initialize user-level CPU registers, before jumping to user code

        void SaveState();    // Save/restore address space-specific
        void RestoreState() const; // info on a context switch

        [[nodiscard]] unsigned int GetNumPages() const { return numPages; }

        /**
         * @brief Extend the heap by n pages
         *
         * @param n Number of pages to allocate (can be 0 to query current brk)
         * @return Pointer to the start of newly allocated memory, or -1 on error
         *
         * Error cases:
         *   - Not enough physical frames available
         *   - Heap would collide with stack area
         *   - n is negative
         */
        int Sbrk(int n);

        [[nodiscard]] unsigned int GetBrk() const { return brk; }
        [[nodiscard]] unsigned int GetHeapStart() const { return heapStart; }
        [[nodiscard]] unsigned int GetHeapSize() const { return brk - heapStart; }

        int SemaphoreCreate(int initialValue);
        [[nodiscard]] int SemaphoreWait(int semId) const;
        [[nodiscard]] int SemaphorePost(int semId) const;
        [[nodiscard]] int SemaphoreDestroy(int semId) const;

        int AllocateSemaphoreTable(unsigned int maxSem);

    private:
        TranslationEntry *pageTable; // Assume linear page table translation for now!
        unsigned int numPages; // Number of pages in the virtual address space
        unsigned int maxPages;

        unsigned int heapStart = 0;
        unsigned int brk = 0;
        unsigned int stackLimit = 0;

        unsigned int maxSemaphores = 0;
        BitMapThreadSafe* semaphoreBitmap = nullptr;
        semaphore_descriptor* semaphoreTable = nullptr;

        [[nodiscard]] class Semaphore* GetSemaphore(int semId)const;

        [[nodiscard]] bool ExtendPageTable(unsigned int newNumPages) const;
};

#endif // ADDRSPACE_H
