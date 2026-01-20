// addrspace.cc
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "addrspace.h"

#include "bitmap.h"
#include "copyright.h"
#include "frameprovider.h"
#include "noff.h"
#include "process.h"
#include "stackmanager.h"
#include "synch.h"
#include "system.h"
#include "nos_tls.h"
#include "kernelpanic.h"

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader(NoffHeader* noffH) {
    noffH->noffMagic = static_cast<int>(WordToHost(noffH->noffMagic));
    noffH->code.size = static_cast<int>(WordToHost(noffH->code.size));
    noffH->code.virtualAddr = static_cast<int>(WordToHost(noffH->code.virtualAddr));
    noffH->code.inFileAddr = static_cast<int>(WordToHost(noffH->code.inFileAddr));
    noffH->initData.size = static_cast<int>(WordToHost(noffH->initData.size));
    noffH->initData.virtualAddr = static_cast<int>(WordToHost(noffH->initData.virtualAddr));
    noffH->initData.inFileAddr = static_cast<int>(WordToHost(noffH->initData.inFileAddr));
    noffH->uninitData.size = static_cast<int>(WordToHost(noffH->uninitData.size));
    noffH->uninitData.virtualAddr = static_cast<int>(WordToHost(noffH->uninitData.virtualAddr));
    noffH->uninitData.inFileAddr = static_cast<int>(WordToHost(noffH->uninitData.inFileAddr));
}

static void ReadAtVirtual(OpenFile* executable, const int virtualaddr, const int numBytes, const int position, TranslationEntry* pageTable, const unsigned int numPages) {
    // Don't remove these lines
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
    const auto tmp = new char[numBytes + 1];
    const int maxSize = executable->ReadAt(tmp, numBytes,  position);
    for (int i = 0; i < maxSize; i++){ 
        machine->WriteMem(virtualaddr + i , 1, tmp[i]);
    }
    delete[] tmp;
}

AddrSpace::AddrSpace(inode_t inode) {
    NoffHeader noffH;

    // Read in the file header
    OpenFile* executable = fileSystem->getFileWithInode(inode);
    executable->ReadAt(reinterpret_cast<char *>(&noffH), sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC)) {
        SwapHeader(&noffH);
    }
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // Calculate end of static data (code + data + bss)
    unsigned int endOfStaticData = 0;
    if (noffH.uninitData.size > 0) { endOfStaticData = noffH.uninitData.virtualAddr + noffH.uninitData.size; }
    else if (noffH.initData.size > 0) { endOfStaticData = noffH.initData.virtualAddr + noffH.initData.size; }
    else { endOfStaticData = noffH.code.virtualAddr + noffH.code.size; }

    heapStart = divRoundUp(endOfStaticData, PageSize) * PageSize;
    DEBUG('a', "AddrSpace::AddrSpace: End of static data at 0x%x, heap starts at 0x%x\n", endOfStaticData, heapStart);

    const unsigned int staticPages = divRoundUp(endOfStaticData, PageSize);
    constexpr unsigned int heapPages = INITIAL_HEAP_PAGES;
    constexpr unsigned int stackPages = divRoundUp(UserStackSize, PageSize);

    numPages = staticPages + MAX_HEAP_PAGES + stackPages;
    stackLimit = (staticPages + MAX_HEAP_PAGES) * PageSize;
    codeStart = noffH.code.virtualAddr;
    codeSize = noffH.code.size;

    DEBUG('a', "AddrSpace::AddrSpace: staticPages=%u, heapPages=%u, stackPages=%u, total=%u\n", staticPages, heapPages, stackPages, numPages);
    DEBUG('a', "AddrSpace::AddrSpace: stackLimit=0x%x\n", stackLimit);

    const unsigned int size = numPages * PageSize;

    ASSERT(staticPages + heapPages + stackPages <= NumPhysPages); // Check we can fit all into physical memory

    // Set up the page table
    DEBUG('a', "AddrSpace::AddrSpace: Initializing address space, num pages %d, size %d\n", numPages, size);
    pageTable = new TranslationEntry[numPages];

    // Initialize all entries as invalid first
    for (unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = 0;
        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
    }

    // Allocate frames for static (code + data + bss) and for heap pages (these two are contiguous)
    for (unsigned int i = 0; i < staticPages + heapPages; i++) {
        const int physPage = frameProvider->GetEmptyFrame();
        if (physPage == -1) {
            DEBUG('a', "AddrSpace::AddrSpace: Unable to allocate frame for static page %d\n", i);
            for (unsigned int j = 0; j < i; j++) { if (pageTable[j].valid) { frameProvider->ReleaseFrame(static_cast<int>(pageTable[j].physicalPage)); } }
            delete[] pageTable;
            pageTable = nullptr;
            numPages = 0;
            DEBUG('a', "AddrSpace::AddrSpace: Failed to allocate memory, address space invalid\n");
            return;
        }
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
    }

    // Allocate frames for stack pages
    // TODO: Consider allocating stack pages on demand instead of all at once
    for (unsigned int i = stackLimit / PageSize; i < numPages; i++) {
        const int physPage = frameProvider->GetEmptyFrame();
        if (physPage == -1) {
            DEBUG('a', "AddrSpace::AddrSpace: Unable to allocate frame for stack page %d\n", i);
            for (unsigned int j = 0; j < i; j++) { if (pageTable[j].valid) { frameProvider->ReleaseFrame(static_cast<int>(pageTable[j].physicalPage)); } }
            delete[] pageTable;
            pageTable = nullptr;
            numPages = 0;
            DEBUG('a', "AddrSpace::AddrSpace: Failed to allocate stack memory, address space invalid\n");
            return;
        }
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
    }

    // then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "AddrSpace::AddrSpace: Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);
        ReadAtVirtual(executable, noffH.code.virtualAddr, noffH.code.size, noffH.code.inFileAddr, pageTable, numPages);
        for (unsigned int j = 0; j < divRoundUp(codeSize, PageSize) - 1; j++){
            pageTable[j].readOnly = true; // TODO we don't protect the last page of code because data can be a little inside think about it
        }
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "AddrSpace::AddrSpace: Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
        ReadAtVirtual(executable, noffH.initData.virtualAddr, noffH.initData.size, noffH.initData.inFileAddr, pageTable, numPages);
    }

    brk = heapStart + (heapPages * PageSize);
    DEBUG('a', "AddrSpace::AddrSpace: Initial brk set to 0x%x (%u heap pages)\n", brk, heapPages);

    stackManager = new StackManager(this, numPages * PageSize, stackLimit, MAX_THREAD);

    AllocateSemaphoreTable(INITIAL_SEMAPHORE_TABLE_SIZE);
}

AddrSpace::~AddrSpace() {
    for (unsigned int i = 0; i < numPages; i++) {
        if (pageTable[i].valid) {
            frameProvider->ReleaseFrame(static_cast<int>(pageTable[i].physicalPage));
        }
    }

    // LB: Missing [] for delete
    // delete pageTable;
    delete[] pageTable;
    // End of modification

    delete semaphoreBitmap;
    delete semaphoreBitMapLock;
    for (unsigned int i = 0; i < maxSemaphores; i++) {
        if (semaphoreTable[i].valid) {
            delete semaphoreTable[i].semaphore;
        }
    }
    delete[] semaphoreTable;
    delete stackManager;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters() const {

    for (int i = 0; i < NumTotalRegs; i++) { machine->WriteRegister(i, 0); }

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, static_cast<int>(numPages) * PageSize - 16);
    DEBUG('a', "AddrSpace::InitRegisters: Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() const {
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

int AddrSpace::Sbrk(const int n) {
    if (n == 0) { return static_cast<int>(brk); }

    if (n < 0) {
        DEBUG('a', "AddrSpace::Sbrk: Shrinking heap not supported (n=%d)\n", n);
        return -1;
    }

    const auto pagesNeeded = static_cast<unsigned int>(n);
    const unsigned int newBrk = brk + (pagesNeeded * PageSize);

    if (newBrk > stackLimit) {
        DEBUG('a', "AddrSpace::Sbrk: Heap would collide with stack (newBrk=0x%x > stackLimit=0x%x)\n", newBrk, stackLimit);
        return -1;
    }

    if (frameProvider->NumAvailFrame() < static_cast<int>(pagesNeeded)) {
        DEBUG('a', "AddrSpace::Sbrk: Not enough frames (need %u, have %d)\n", pagesNeeded, frameProvider->NumAvailFrame());
        return -1;
    }

    const unsigned int startPage = brk / PageSize;
    const unsigned int endPage = startPage + pagesNeeded;

    DEBUG('a', "AddrSpace::Sbrk: Allocating pages %u to %u (brk: 0x%x -> 0x%x)\n", startPage, endPage - 1, brk, newBrk);

    for (unsigned int i = startPage; i < endPage; i++) {
        if (pageTable[i].valid) { continue; }

        const int physPage = frameProvider->GetEmptyFrame();
        if (physPage == -1) {
            DEBUG('a', "AddrSpace::Sbrk: Failed to allocate frame for page %u\n", i);
            return -1;
        }

        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
    }

    const unsigned int oldBrk = brk;
    brk = newBrk;
    DEBUG('a', "AddrSpace::Sbrk: Success, returning 0x%x (new brk=0x%x, numPages=%u)\n", oldBrk, brk, numPages);

    return static_cast<int>(oldBrk);
}

unsigned int AddrSpace::GetStackBottom() const {
    if (stackManager != nullptr) {
        return stackManager->GetStackAreaBottom();
    }
    return brk;
}

int AddrSpace::SemaphoreCreate(const int initialValue) {
    semaphoreBitMapLock->Acquire();
    int semId = semaphoreBitmap->Find();
    semaphoreBitMapLock->Release();
    if (semId == -1) {
        DEBUG('c', "AddrSpace::SemaphoreCreate: Failed to create semaphore, table full\n");
        AllocateSemaphoreTable(maxSemaphores * 2); // Double the size of the semaphore table
        semaphoreBitMapLock->Acquire();
        semId = semaphoreBitmap->Find();
        semaphoreBitMapLock->Release();
        if (semId == -1) {
            DEBUG('c', "AddrSpace::SemaphoreCreate: Failed to create semaphore even after resizing table\n");
            return -1;
        }
    }
    semaphoreTable[semId].semaphore = new Semaphore("UserSemaphore", initialValue);
    semaphoreTable[semId].valid = true;
    DEBUG('c', "AddrSpace::SemaphoreCreate: Created semaphore with id %d and initial value %d\n", semId, initialValue);
    return semId;
}

int AddrSpace::SemaphoreWait(const int semId) const {
    Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    sem->P();
    DEBUG('c', "AddrSpace::SemaphoreWait: Semaphore with id %d waited (P operation)\n", semId);
    return 0;
}

int AddrSpace::SemaphorePost(const int semId) const {
    Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    sem->V();
    DEBUG('c', "AddrSpace::SemaphorePost: Semaphore with id %d posted (V operation)\n", semId);
    return 0;
}

int AddrSpace::SemaphoreDestroy(const int semId) const {
    const Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    delete sem;
    semaphoreTable[semId].semaphore = nullptr;
    semaphoreTable[semId].valid = false;
    semaphoreBitMapLock->Acquire();
    semaphoreBitmap->Clear(semId);
    semaphoreBitMapLock->Release();
    DEBUG('c', "AddrSpace::SemaphoreDestroy: Semaphore with id %d destroyed\n", semId);
    return 0;
}

Semaphore* AddrSpace::GetSemaphore(const int semId) const {
    if (semId < 0 || semId >= MAX_SEMAPHORES_PER_PROCESS || !semaphoreTable[semId].valid) {
        return nullptr;
    }
    return semaphoreTable[semId].semaphore;
}

/*----------------------------------------------------------------------
 * AddrSpace::AllocateSemaphoreTable
 *      Allocate or reallocate the semaphore table to hold up to maxSem semaphores.
 *      If the table already exists, don't clear its contents.
 *      If the new size is lower than the old size, all entries beyond the new size are lost,
 *      so it is not recommended to reduce the size of the semaphore table.
 *----------------------------------------------------------------------*/
int AddrSpace::AllocateSemaphoreTable(const unsigned int maxSem) {
    DEBUG('c', "AddrSpace::AllocateSemaphoreTable: Allocating semaphore table with size %u\n", maxSem);
    if (maxSem > MAX_SEMAPHORES_PER_PROCESS) { return -1; }
    if (maxSem == this->maxSemaphores) { return 0; } // No change needed

    if (this->semaphoreBitmap != nullptr) {
        semaphoreBitMapLock->Acquire();
        this->semaphoreBitmap->UpdateSize(static_cast<int>(maxSem));
        semaphoreBitMapLock->Release();
    }
    else {
        this->semaphoreBitmap = new BitMap(static_cast<int>(maxSem));
        this->semaphoreBitMapLock = new Lock("SemaphoreBitMapLock");
    }

    const semaphore_descriptor* oldTable = this->semaphoreTable;
    this->semaphoreTable = new semaphore_descriptor[maxSem];

    if (oldTable != nullptr) {
        // Copy old entries to the new table
        for (unsigned int i = 0; i < this->maxSemaphores && i < maxSem; i++) {
            this->semaphoreTable[i] = oldTable[i];
        }
        // Initialize new entries
        for (unsigned int i = this->maxSemaphores; i < maxSem; i++) {
            this->semaphoreTable[i].semaphore = nullptr;
            this->semaphoreTable[i].valid = false;
        }
        delete[] oldTable;
    } else {
        // Initialize all entries
        for (unsigned int i = 0; i < maxSem; i++) {
            this->semaphoreTable[i].semaphore = nullptr;
            this->semaphoreTable[i].valid = false;
        }
    }

    DEBUG('c', "AddrSpace::AllocateSemaphoreTable: Successfully allocated semaphore table with size %u\n", maxSem);

    this->maxSemaphores = maxSem;
    return 0;
}

bool AddrSpace::IsUserAddress(const unsigned int addr) const {
    return addr < numPages * PageSize;
}

bool AddrSpace::IsValidUserRange(const unsigned int addr, const unsigned int size) const {
    if (size == 0) { return true; }
    if (addr + size < addr) { return false; } // overflow protection
    if (const unsigned int end = addr + size - 1; !IsUserAddress(addr) || !IsUserAddress(end)) { return false; }
    return true;
}

bool AddrSpace::IsInCodeSegment(const unsigned int addr) const {
    return addr >= codeStart && addr <  codeStart + codeSize;
}

bool AddrSpace::IsInHeap(const unsigned int addr) const {
    return addr >= heapStart && addr < brk;
}

bool AddrSpace::IsInStackArea(const unsigned int addr) const {
    return addr >= stackLimit && addr < GetStackTop();
}

bool AddrSpace::IsValidTLS(const unsigned int tlsAddr) const {
    if (tlsAddr == 0) { return true; } // TLS is optional
    if (!IsValidUserRange(tlsAddr, TLS_TOTAL_SIZE)) { return false; }
    if (!IsAligned(tlsAddr, 4)) { return false; }
    return true;
}
