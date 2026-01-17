// file.cc 

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "file.h"
#include "fileindirection.h"

File::File() {
}

void File::FetchFrom(sector_t sector) const {
    ASSERT(sector != -1);
    synchDisk->ReadSector(sector, (char *)(this));
}

void File::WriteBack(sector_t sector) const {
    synchDisk->WriteSector(sector, (char *)(this));
}

void File::Print(){

    indirect.Print();
}

