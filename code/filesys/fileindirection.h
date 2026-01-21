#ifndef FILEENTRY_H
#define FILEENTRY_H
#include "fileconst.h"
#include "filetype.h"
#include "system.h"

// The following class defines a "file entry", representing an indirection sector 
// in the file.  Each entry gives the sector of the data
//
// Internal data structures kept public so that File operations can
// access them directly.

#define MAX_INDIRECT_LEVEL_ONE 32
#define MAX_INDIRECT_LEVEL_TWO 31 //(SectorSize - MAX_INDIRECT_LEVEL_ONE * 4) / 4

class FirstIndirection {

    public:
        FirstIndirection();

        void FetchFrom(const int sectorNumber);
        void WriteAt(const int sectorNumber);
        void Print(int far_from_data = 0);
        void setSector(int index, sector_t sector);
        sector_t getSector(int index);
        int getLastUse(){
            for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
                if (entries[i] != INVALID_SECTOR){return i;}
            }
            return INVALID_SECTOR;
        }
        bool InUse(int index);
        int getNumberFree();
        //BITLMPA

        sector_t entries[MAX_INDIRECT_LEVEL_ONE];
};

typedef FirstIndirection SecondIndirection;

static_assert(sizeof(FirstIndirection) == SectorSize);


#endif // FILEENTRY_H
