#include "stackmanager.h"

#include "addrspace.h"
#include "nos_errno.h"
#include "nos_tls.h"
#include "../machine/machine.h"
#include "../threads/synch.h"
#include "../threads/utility.h"
#include "kernelpanic.h"

StackManager::StackManager(AddrSpace* space, unsigned int top, unsigned int bottom, unsigned int max)
    : addrSpace(space),
      stackAreaTop(top),
      stackAreaBottom(bottom),
      nextStackTop(top),
      maxStacks(max),
      allocatedCount(0),
      regions(new StackRegion[max]),
      lock(new Lock("StackManager Lock")) {
    ASSERT_KP(top > bottom);
    DEBUG('s', "StackManager::StackManager: created with top=0x%x, bottom=0x%x, max=%d\n", stackAreaTop, stackAreaBottom, maxStacks);
}

StackManager::~StackManager() {
    delete[] regions;
    delete lock;
}

unsigned int StackManager::RoundUpToPage(unsigned int size) {
    return ((size + PageSize - 1) / PageSize) * PageSize;
}

int StackManager::FindFreeSlot() const {
    for (unsigned int i = 0; i < maxStacks; i++) {
        if (!regions[i].inUse) {
            regions[i] = StackRegion(); // Clear previous data
            return static_cast<int>(i);
        }
    }
    return -1;
}

int StackManager::FindByBase(const unsigned int base) const {
    for (unsigned int i = 0; i < maxStacks; i++) {
        if (regions[i].inUse && regions[i].base == base) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// TODO: Real page allocation for stacks, to avoid full stack allocation at beginning
int StackManager::AllocateStack(const unsigned int size, unsigned int *base, unsigned int *limit) {
    lock->Acquire();

    const int slot = FindFreeSlot();
    if (slot < 0) {
        lock->Release();
        DEBUG('s', "StackManager::AllocateStack: no free slots (max=%d)\n", maxStacks);
        return -E_THREAD_LIMIT;
    }

    unsigned int alignedSize = RoundUpToPage(size);
    if (alignedSize < USER_STACK_MIN_SIZE) { alignedSize = USER_STACK_MIN_SIZE; }
    if (alignedSize > USER_STACK_MAX_SIZE) { alignedSize = USER_STACK_MAX_SIZE; }

    const unsigned int newBase = nextStackTop;
    const unsigned int newLimit = newBase - alignedSize;

    if (newLimit < stackAreaBottom || newLimit > newBase) {
        lock->Release();
        DEBUG('s', "StackManager::AllocateStack: out of space (need 0x%x, available 0x%x)\n", alignedSize, nextStackTop - stackAreaBottom);
        return -E_NOMEM;
    }

    regions[slot] = StackRegion(newBase, newLimit, true, false, 0);

    nextStackTop = newLimit;
    allocatedCount++;

    *base = newBase;
    *limit = newLimit;

    DEBUG('s', "StackManager::AllocateStack: allocated stack [0x%x - 0x%x], size=%d, slot=%d\n", newLimit, newBase, alignedSize, slot);

    lock->Release();
    return 0;
}

int StackManager::RegisterUserStack(const unsigned int base, const unsigned int size, unsigned int *limit) {
    lock->Acquire();

    const int slot = FindFreeSlot();
    if (slot < 0) {
        lock->Release();
        return -E_THREAD_LIMIT;
    }

    const unsigned int alignedSize = RoundUpToPage(size);
    const unsigned int computedLimit = base - alignedSize;

    if (base > stackAreaTop || computedLimit < stackAreaBottom) {
        lock->Release();
        DEBUG('s', "StackManager::RegisterUserStack: user stack out of bounds\n");
        return -E_STACK_ADDR;
    }

    // Check for overlaps with existing stacks
    for (unsigned int i = 0; i < maxStacks; i++) {
        if (regions[i].inUse) {
            if (!(base <= regions[i].limit || computedLimit >= regions[i].base)) {
                lock->Release();
                DEBUG('s', "StackManager::RegisterUserStack: user stack overlaps with existing stack\n");
                return -E_STACK_ADDR;
            }
        }
    }

    regions[slot].base = base;
    regions[slot].limit = computedLimit;
    regions[slot].inUse = true;
    regions[slot].userProvided = true;
    regions[slot].threadTid = 0;

    allocatedCount++;
    *limit = computedLimit;

    DEBUG('s', "StackManager::RegisterUserStack: registered user stack [0x%x - 0x%x]\n", computedLimit, base);

    lock->Release();
    return 0;
}

int StackManager::FreeStack(const unsigned int base) {
    lock->Acquire();

    const int slot = FindByBase(base);
    if (slot < 0) {
        lock->Release();
        DEBUG('s', "StackManager::FreeStack: base 0x%x not found\n", base);
        return -E_INVAL;
    }

    DEBUG('s', "StackManager::FreeStack: freeing stack at 0x%x (slot %d, userProvided=%d)\n", base, slot, regions[slot].userProvided);

    const unsigned int freedLimit = regions[slot].limit;

    // Mark as free
    regions[slot].inUse = false;
    allocatedCount--;

    // TODO: Better freeing gestion
    if (freedLimit == nextStackTop) {
        nextStackTop = regions[slot].base;
        DEBUG('s', "StackManager::FreeStack: adjusted nextStackTop to 0x%x\n", nextStackTop);
    }

    lock->Release();
    return 0;
}

void StackManager::MarkInUse(const unsigned int base, const unsigned int tid) const {
    lock->Acquire();

    if (const int slot = FindByBase(base); slot >= 0) {
        regions[slot].threadTid = tid;
    }

    lock->Release();
}

bool StackManager::GetStackInfo(unsigned int base, StackRegion &outRegion) const{
    lock->Acquire();

    if (const int slot = FindByBase(base); slot >= 0) {
        outRegion = regions[slot];
        lock->Release();
        return true;
    }

    lock->Release();
    return false;
}

bool StackManager::IsInStack(const unsigned int addr) const {
    lock->Acquire();

    for (unsigned int i = 0; i < maxStacks; i++) {
        if (regions[i].inUse) {
            if (addr >= regions[i].limit && addr < regions[i].base) {
                lock->Release();
                return true;
            }
        }
    }

    lock->Release();
    return false;
}

bool StackManager::IsValidUserStackPointer(const unsigned int sp) const {
    if (sp < stackAreaBottom || sp > stackAreaTop) { return false; }
    if (!IsAligned(sp, 4)) { return false; }
    return true;
}

bool StackManager::IsKnownStackBase(const unsigned int base) const {
    return FindByBase(base) >= 0;
}

void StackManager::Print() {
    lock->Acquire();

    printf("StackManager: %d/%d stacks allocated\n", allocatedCount, maxStacks);
    printf("  Stack area: 0x%x - 0x%x\n", stackAreaBottom, stackAreaTop);
    printf("  Next stack top: 0x%x\n", nextStackTop);
    printf("  Regions:\n");

    for (unsigned int i = 0; i < maxStacks; i++) {
        if (regions[i].inUse) {
            printf("    [%d] base=0x%x limit=0x%x size=%d tid=%d %s\n",
                   i, regions[i].base,
                   regions[i].limit,
                   regions[i].base - regions[i].limit,
                   regions[i].threadTid,
                   regions[i].userProvided ? "(user)" : "(kernel)");
        }
    }

    lock->Release();
}