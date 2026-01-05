#include "frameprovider.h"
#include "bitmap.h"
#include "system.h"

int SequentialAllocationStrategy::SelectFrame(BitMap* bitmap, unsigned int numFrames) {
    return bitmap->Find();
}

// Very bad performance, for test purpose only
int RandomAllocationStrategy::SelectFrame(BitMap* bitmap, unsigned int numFrames) {
    for (unsigned int i = 0; i < numFrames; ++i) {
        int frame = rand() % numFrames;
        if (!bitmap->Test(frame)) {
            bitmap->Mark(frame);
            return frame;
        }
    }
    return -1;
}

FrameProvider::FrameProvider(IAllocationStrategy* strategy) : numFrames(NumPhysPages) {
    bitmap = new BitMap(numFrames);

    if (strategy != nullptr) { allocationStrategy = strategy; }
	else { allocationStrategy = new SequentialAllocationStrategy(); }

    DEBUG('a', "FrameProvider: Initialized with %u frames.\n", numFrames);
}

FrameProvider::~FrameProvider() {
    delete bitmap;
    delete allocationStrategy;
}

int FrameProvider::GetEmptyFrame() {
    int frameNumber = allocationStrategy->SelectFrame(bitmap, numFrames);

    if (frameNumber == -1) [[unlikely]] {
        DEBUG('a', "Frame Provider: No available frames!\n");
        return -1;
    }

    ASSERT(frameNumber >= 0 && frameNumber < (int)numFrames);

    char* frameAddress = &(machine->mainMemory[frameNumber * PageSize]);
    bzero(frameAddress, PageSize);

    DEBUG('a', "Frame Provider: Allocated frame %d at physical address 0x%X.\n", frameNumber, (unsigned int)frameAddress);

    return frameNumber;
}

void FrameProvider::SetAllocationStrategy(IAllocationStrategy* strategy) {
    if (strategy == nullptr) {
        DEBUG('a', "Frame Provider: Cannot set null allocation strategy.\n");
        return;
    }

	delete allocationStrategy;
    allocationStrategy = strategy;
    DEBUG('a', "Frame Provider: Allocation strategy updated.\n");
}

void FrameProvider::ReleaseFrame(int frameNumber) {
    ASSERT(frameNumber >= 0 && frameNumber < (int)numFrames);

    bitmap->Clear(frameNumber);

    DEBUG('a', "Frame Provider: Released frame %d.\n", frameNumber);
}

int FrameProvider::NumAvailFrame() {
    return bitmap->NumClear();
}


