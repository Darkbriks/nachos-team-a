#include "fileindirection.h"

FirstIndirection::FirstIndirection(){
    for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
        setSector(i, INVALID_SECTOR);
    }
}
            
void FirstIndirection::setSector(int index, sector_t sector){
    entries[index] = sector;
}

bool FirstIndirection::InUse(int index){
    if (index < 0 || index > MAX_INDIRECT_LEVEL_ONE){
        return false;
    }
    return entries[index] != INVALID_SECTOR;

}

sector_t FirstIndirection::getSector(int index){
    if (index < 0 || index > MAX_INDIRECT_LEVEL_ONE){
        return INVALID_SECTOR;
    }
    return entries[index];
}

void FirstIndirection::FetchFrom(const int sectorNumber) {
    synchDisk->ReadSector(sectorNumber, reinterpret_cast<char *>(this));
}

void FirstIndirection::WriteAt(const int sectorNumber) {
    synchDisk->WriteSector(sectorNumber, reinterpret_cast<char *>(this));
}

void FirstIndirection::Print(){
    for (int i = 0; i < MAX_INDIRECT_LEVEL_ONE; i++){
        if (InUse(i)){
            printf("sector %d is used and stored on entry %d\n", getSector(i), i);
        } else {
            printf("entry %d is not used\n", i);
        }
    }
}
