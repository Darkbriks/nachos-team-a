#include "fileindirection.h"

FirstIndirection::FirstIndirection(){nbEntry = MAX_INDIRECT_LEVEL_ONE;
            for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
                entries[i].inUse = false;
            }
        }

void FirstIndirection::FetchFrom(const int sectorNumber) {
    synchDisk->ReadSector(sectorNumber, reinterpret_cast<char *>(this));
}

void FirstIndirection::WriteAt(const int sectorNumber) {
    synchDisk->WriteSector(sectorNumber, reinterpret_cast<char *>(this));
}

void FirstIndirection::Print(){
    for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
        if (! entries[i].InUse()){
            printf("sector %d is not used\n", entries[i].getSector());
        } else {
            printf("sector %d is used\n", entries[i].getSector());
        }
    }
}
