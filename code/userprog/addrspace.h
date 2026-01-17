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
#include "machine.h"
#include "translate.h"

#define UserStackSize (8196*4) // increase this as necessary!

#define USER_STACK_MIN_SIZE (2 * PageSize)
#define USER_STACK_DEFAULT_SIZE (8 * PageSize)
#define USER_STACK_MAX_SIZE (64 * PageSize)

#define INITIAL_SEMAPHORE_TABLE_SIZE 16
#define MAX_SEMAPHORES_PER_PROCESS 512 // Arbitrary limit, can be adjusted as needed

#define INITIAL_HEAP_PAGES 2
#define MAX_HEAP_PAGES  256

class BitMapThreadSafe;
class StackManager;
class Process;

struct semaphore_descriptor {
    class Semaphore* semaphore;
    bool valid;
};

static inline bool IsAligned(const unsigned int addr, const unsigned int align) {
    return (addr & (align - 1)) == 0;
}

class AddrSpace {
    public:
        /**
         * @brief Create an address space for a user program
         *
         * Loads the program from the given executable file in NOFF format,
         * sets up the page table, and allocates physical frames for
         * the static segments (code, data, bss) and initial heap and stack.
         *
         * Memory scheme:
         * | code segment | data segment | bss segment | heap | ... free ... | stack |
         *
         * @param executable The executable file in NOFF format
         */
        explicit AddrSpace(OpenFile *executable);

        /**
         * @brief De-allocate an address space
         *
         * Release all physical frames used by this address space,
         * and clean up allocated data structures.
         */
        ~AddrSpace();

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

        [[nodiscard]] StackManager* GetStackManager() const { return stackManager; }
        [[nodiscard]] unsigned int GetStackTop() const { return numPages * PageSize; }
        [[nodiscard]] unsigned int GetStackBottom() const;

        int SemaphoreCreate(int initialValue);
        [[nodiscard]] int SemaphoreWait(int semId) const;
        [[nodiscard]] int SemaphorePost(int semId) const;
        [[nodiscard]] int SemaphoreDestroy(int semId) const;

        int AllocateSemaphoreTable(unsigned int maxSem);

        [[nodiscard]] bool IsUserAddress(unsigned int addr) const;
        [[nodiscard]] bool IsValidUserRange(unsigned int addr, unsigned int size) const;
        [[nodiscard]] bool IsInCodeSegment(unsigned int addr) const;
        [[nodiscard]] bool IsInHeap(unsigned int addr) const;
        [[nodiscard]] bool IsInStackArea(unsigned int addr) const;
        [[nodiscard]] bool IsValidTLS(unsigned int tlsAddr) const;

        [[nodiscard]] bool IsValid() const { return pageTable != nullptr; }

    private:
        TranslationEntry *pageTable; // Assume linear page table translation for now!
        unsigned int numPages; // Number of pages in the virtual address space

        unsigned int codeStart = 0;
        unsigned int codeSize = 0;
        unsigned int heapStart = 0;
        unsigned int brk = 0;
        unsigned int stackLimit = 0;

        StackManager* stackManager = nullptr;

        unsigned int maxSemaphores = 0;
        BitMapThreadSafe* semaphoreBitmap = nullptr;
        semaphore_descriptor* semaphoreTable = nullptr;

        [[nodiscard]] class Semaphore* GetSemaphore(int semId)const;
};

#endif // ADDRSPACE_H
