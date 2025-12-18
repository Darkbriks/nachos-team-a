#ifndef BITMAP_THREAD_SAFE_H
#define BITMAP_THREAD_SAFE_H

#include "bitmap.h"



class BitMapThreadSafe : public BitMap{

    public:
        BitMapThreadSafe(int nitems); // Initialize a bitmap, with "nitems" bits
                            // initially, all bits are cleared.
        ~BitMapThreadSafe() override;          // De-allocate bitmap

        void Mark(int which)override ;  // Set the "nth" bit
        void Clear(int which)override ; // Clear the "nth" bit
        bool TestThreadSafe(int which);  // Is the "nth" bit set?
        int Find()override ;            // Return the # of a clear bit, and as a side
        // effect, set the bit.
        // If no bits are clear, return -1.
        int NumClearThreadSafe() ; // Return the number of clear bits


        // These aren't needed until FILESYS, when we will need to read and
        // write the bitmap to a file
        // TODO when FILESYS
        // void FetchFrom(OpenFile *file)override ; // fetch contents from disk
        // void WriteBack(OpenFile *file) override; // write contents to disk
    

    private:

        class Semaphore *semaphore;

};

#endif // BITMAP_THREAD_SAFE_H
