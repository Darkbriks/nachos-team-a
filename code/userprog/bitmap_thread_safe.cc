#include "bitmap_thread_safe.h"
#include "synch.h"


BitMapThreadSafe::BitMapThreadSafe(int nitems) : BitMap(nitems){
    semaphore = new Semaphore("bit_Map", 1);
}

void BitMapThreadSafe::Mark(int which){
    semaphore->P();
    BitMap::Mark(which);
    semaphore->V();
}

void BitMapThreadSafe::Clear(int which){
    semaphore->P();
    BitMap::Clear(which);
    semaphore->V();
}

bool BitMapThreadSafe::TestThreadSafe(int which){
    semaphore->P();
    bool result = BitMap::Test(which);
    semaphore->V();
    return result;
}

int BitMapThreadSafe::Find(){
    semaphore->P();
    for (int i = 0; i < numBits; i++)
        if (!Test(i)) {
            BitMap::Mark(i);
            semaphore->V();
            return i;
        }
    semaphore->V();
    return -1;
}

int BitMapThreadSafe::NumClearThreadSafe(){
    semaphore->P();
    int result = BitMap::NumClear();
    semaphore->V();
    return result;
}

BitMapThreadSafe::~BitMapThreadSafe(){
    BitMap::~BitMap();
    delete semaphore;
}
