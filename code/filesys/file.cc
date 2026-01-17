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

void File::Print() const {

    for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
        if (! indirect.entries[i].InUse()){
            printf("sector %d is not used\n", indirect.entries[i].getSector());
        } else {
            printf("sector %d is used\n", indirect.entries[i].getSector());
        }
    }
}

