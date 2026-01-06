#include "frameprovider.h"
#include "bitmap.h"
#include "system.h"
#include "synch.h"

Semaphore *sem = new Semaphore("Frame_provider", 1);

int SequentialAllocationStrategy::SelectFrame(BitMap* bitmap, unsigned int numFrames) {
    sem->P();
    int x = bitmap->Find();
    sem->V();
    return x;
}

// Very bad performance, for test purpose only
// Can return -1 and memory is available. Need to memorize already seens number ?
int RandomAllocationStrategy::SelectFrame(BitMap* bitmap, unsigned int numFrames) {
    sem->P();
    for (unsigned int i = 0; i < numFrames; ++i) {
        int frame = rand() % numFrames;
        if (!bitmap->Test(frame)) {
            bitmap->Mark(frame);
            sem->V();
            return frame;
        }
    }
    sem->V();
    return -1;
}

FrameProvider::FrameProvider(IAllocationStrategy* strategy) : numFrames(NumPhysPages) {
    bitmap = new BitMap(numFrames);

    if (strategy != nullptr) { allocationStrategy = strategy; }
	else { allocationStrategy = new SequentialAllocationStrategy(); }

    DEBUG('f', "FrameProvider: Initialized with %u frames.\n", numFrames);
}

FrameProvider::~FrameProvider() {
    delete bitmap;
    delete allocationStrategy;
}

int FrameProvider::GetEmptyFrame() {
    int frameNumber = allocationStrategy->SelectFrame(bitmap, numFrames);

    if (frameNumber == -1) [[unlikely]] {
        DEBUG('f', "Frame Provider: No available frames!\n");
        return -1;
    }

    ASSERT(frameNumber >= 0 && frameNumber < (int)numFrames);

    char* frameAddress = &(machine->mainMemory[frameNumber * PageSize]);
    bzero(frameAddress, PageSize);

    DEBUG('f', "Frame Provider: Allocated frame %d at physical address 0x%X.\n", frameNumber, (unsigned int)frameAddress);

    return frameNumber;
}

void FrameProvider::SetAllocationStrategy(IAllocationStrategy* strategy) {
    if (strategy == nullptr) {
        DEBUG('f', "Frame Provider: Cannot set null allocation strategy.\n");
        return;
    }

	delete allocationStrategy;
    allocationStrategy = strategy;
    DEBUG('f', "Frame Provider: Allocation strategy updated.\n");
}

void FrameProvider::ReleaseFrame(int frameNumber) {
    ASSERT(frameNumber >= 0 && frameNumber < (int)numFrames);

    bitmap->Clear(frameNumber);

    DEBUG('f', "Frame Provider: Released frame %d.\n", frameNumber);
}

int FrameProvider::NumAvailFrame() {
    return bitmap->NumClear();
}


