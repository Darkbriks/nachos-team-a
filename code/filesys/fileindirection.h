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

#define MAX_INDIRECT_LEVEL_ONE 15

class FirstIndirection;
class FileEntry {
    friend FirstIndirection;

private:
    bool inUse;				// Is this file entry in use?
    sector_t sector;				// Location on disk to find the data for this file

public:
    bool InUse()const {return inUse;}
    sector_t getSector()const{return sector;}
    void setUse(bool b){inUse = b;}
    void setSector(sector_t s){sector = s;}
};

class FirstIndirection {

    public:
        FirstIndirection();

        void FetchFrom(const int sectorNumber);
        void WriteAt(const int sectorNumber);
        void Print();


        int nbEntry;
        int x;
        FileEntry entries[MAX_INDIRECT_LEVEL_ONE];
};

static_assert(sizeof(FirstIndirection) == SectorSize);

#endif // FILEENTRY_H
