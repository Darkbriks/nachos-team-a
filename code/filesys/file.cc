// file.cc 

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "file.h"
#include "fileindirection.h"

File::File(){
    for (int i = 0; i < MAX_INDIRECT_LEVEL_TWO; i++){
        second_indercetion[i] = INVALID_SECTOR;
    }
}

void File::FetchFrom(sector_t sector) const {
    ASSERT(sector != -1);
    synchDisk->ReadSector(sector, (char *)(this));
}

void File::WriteBack(sector_t sector) const {
    synchDisk->WriteSector(sector, (char *)(this));
}

void File::Print(){
    FirstIndirection * first = new FirstIndirection();
    first->FetchFrom(indirect);
    first->Print();
    for (int i = 0; second_indercetion[i] != INVALID_SECTOR; i++){
        SecondIndirection * second = new SecondIndirection();
        second->FetchFrom(second_indercetion[i]);
        second->Print(1);
    }
}

