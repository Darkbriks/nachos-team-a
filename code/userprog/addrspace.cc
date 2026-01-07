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

#include "bitmap_thread_safe.h"
#include "copyright.h"
#include "frameprovider.h"
#include "noff.h"
#include "synch.h"
#include "system.h"

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
    char tmp[numBytes + 1];
    const int maxSize = executable->ReadAt(tmp, numBytes,  position);
    for (int i = 0; i < maxSize; i++){ 
        machine->WriteMem(virtualaddr + i , 1, tmp[i]);
    }
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) {
    NoffHeader noffH;

    executable->ReadAt(reinterpret_cast<char *>(&noffH), sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC)) {
        SwapHeader(&noffH);
    }
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    unsigned int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize; // we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages); // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (unsigned int i = 0; i < numPages; i++) {
        const int physPage = frameProvider->GetEmptyFrame();
        if (physPage == -1) {
            DEBUG('a', "AddrSpace::AddrSpace: Unable to allocate frame for page %d\n", i);
            ASSERT(FALSE); // TODO: Handle this properly
        }
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE; // if the code segment was entirely on
                                       // a separate page, we could set its
                                       // pages to be read-only
    }

    // then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);
        ReadAtVirtual(executable, noffH.code.virtualAddr, noffH.code.size, noffH.code.inFileAddr, pageTable, numPages);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
        ReadAtVirtual(executable, noffH.initData.virtualAddr, noffH.initData.size, noffH.initData.inFileAddr, pageTable, numPages);
    }

    AllocateSemaphoreTable(INITIAL_SEMAPHORE_TABLE_SIZE);
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

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
    for (unsigned int i = 0; i < maxSemaphores; i++) {
        if (semaphoreTable[i].valid) {
            delete semaphoreTable[i].semaphore;
        }
    }
    delete[] semaphoreTable;
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
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
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

int AddrSpace::SemaphoreCreate(const int initialValue) {
    int semId = semaphoreBitmap->Find();
    if (semId == -1) {
        DEBUG('s', "AddrSpace::SemaphoreCreate: Failed to create semaphore, table full\n");
        AllocateSemaphoreTable(maxSemaphores * 2); // Double the size of the semaphore table
        semId = semaphoreBitmap->Find();
        if (semId == -1) {
            DEBUG('s', "AddrSpace::SemaphoreCreate: Failed to create semaphore even after resizing table\n");
            return -1;
        }
    }
    semaphoreTable[semId].semaphore = new Semaphore("UserSemaphore", initialValue);
    semaphoreTable[semId].valid = true;
    DEBUG('s', "AddrSpace::SemaphoreCreate: Created semaphore with id %d and initial value %d\n", semId, initialValue);
    return semId;
}

int AddrSpace::SemaphoreWait(const int semId) {
    Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    sem->P();
    DEBUG('s', "AddrSpace::SemaphoreWait: Semaphore with id %d waited (P operation)\n", semId);
    return 0;
}

int AddrSpace::SemaphorePost(const int semId) {
    Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    sem->V();
    DEBUG('s', "AddrSpace::SemaphorePost: Semaphore with id %d posted (V operation)\n", semId);
    return 0;
}

int AddrSpace::SemaphoreDestroy(const int semId) {
    const Semaphore* sem = GetSemaphore(semId);
    if (sem == nullptr) {
        return -1;
    }
    delete sem;
    semaphoreTable[semId].semaphore = nullptr;
    semaphoreTable[semId].valid = false;
    semaphoreBitmap->ClearThreadSafe(semId);
    DEBUG('s', "AddrSpace::SemaphoreDestroy: Semaphore with id %d destroyed\n", semId);
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
    DEBUG('s', "AddrSpace::AllocateSemaphoreTable: Allocating semaphore table with size %u\n", maxSem);
    if (maxSem > MAX_SEMAPHORES_PER_PROCESS) { return -1; }
    if (maxSem == this->maxSemaphores) { return 0; } // No change needed

    if (this->semaphoreBitmap != nullptr) { this->semaphoreBitmap->UpdateSize(static_cast<int>(maxSem)); }
    else { this->semaphoreBitmap = new BitMapThreadSafe(static_cast<int>(maxSem)); }

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

    DEBUG('s', "AddrSpace::AllocateSemaphoreTable: Successfully allocated semaphore table with size %u\n", maxSem);

    this->maxSemaphores = maxSem;
    return 0;
}
