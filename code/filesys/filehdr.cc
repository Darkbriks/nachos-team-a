// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "file.h"
#include "fileindirection.h"
#include "system.h"
#include "filehdr.h"

#define MIN(a, b) \
    (unsigned int) a < (unsigned int) b ? a : b

FileHeader::FileHeader(){
    for (unsigned long i = 0; i < NumDirect; i++){
        dataSectors[i] = INVALID_SECTOR;
    }
    setRedirect(INVALID_SECTOR);
    numBytes = 0;
    numSectors = 0;
}

int FileHeader::getDirectInUse(){
    int result = 0;
    for (unsigned long i = 0; i < NumDirect; i++){
        if ( dataSectors[i] != INVALID_SECTOR){
            result++;
        }
    }
    return result;
}


/**
 * @brief Initialize first sectors accesible directly. Allocate no more than numDirect sector
 *
 * @param bitMap  the global bitmap
 * @return  The number of sector allocated 
 */
void FileHeader::initializeDirectData(BitMap *bitMap, int* nb_necessary){
    int inUse = getDirectInUse(); 
    int total_alloc = MIN(*nb_necessary, NumDirect);
    for (int i = inUse; i < total_alloc; i++) {
        dataSectors[i] = bitMap->Find();
    }
    *nb_necessary -= (total_alloc - inUse);
}

File* FileHeader::initializeFirstIndirection(BitMap* bitMap, int* nb_necessary){
    int first_sector_to_write;
    File  * file = new File();
    FirstIndirection * first_indir = new FirstIndirection();

    if (getRedirect() == INVALID_SECTOR){
        first_sector_to_write = 0;
        setRedirect(bitMap->Find());
        file->setFirstIndirection(bitMap->Find());
    } else {
        file->FetchFrom(getRedirect());
        first_indir->FetchFrom(file->getFirstIndirection());
        first_sector_to_write = MAX_INDIRECT_LEVEL_ONE - first_indir->getNumberFree();
    }
    int total_alloc = MIN(*nb_necessary, MAX_INDIRECT_LEVEL_ONE - first_sector_to_write);


    for (int i = first_sector_to_write; i < MAX_INDIRECT_LEVEL_ONE;i++){
        first_indir->setSector(i, bitMap->Find());
    }
    first_indir->WriteAt(file->getFirstIndirection());
    DEBUG('R', "file have sector %d for indirection \n", getRedirect());
    *nb_necessary -= total_alloc;
    return file;
}

void FileHeader::initializeSecondIndirection(BitMap* bitMap, int* nb_necessary, File* file) {
    int total_alloc = MIN(divRoundUp(*nb_necessary, MAX_INDIRECT_LEVEL_ONE), MAX_INDIRECT_LEVEL_TWO);
    int index_in_second = -1;
    SecondIndirection *second = nullptr;
    ASSERT(total_alloc != 0);
    for (int second_indirection_index = 0; second_indirection_index < total_alloc; second_indirection_index++){
        if (second_indirection_index % MAX_INDIRECT_LEVEL_ONE == 0){
            if (second != nullptr){
                DEBUG('R', "file have sector %d saved for indirection 2 at index %d\n", file->getRedirect2(index_in_second), index_in_second);
                second->WriteAt(file->getRedirect2(index_in_second));
                delete second;
                second = nullptr;
            }
            index_in_second++;
            if ( file->getRedirect2(index_in_second) != INVALID_SECTOR){
                second = new SecondIndirection();
                second->FetchFrom(file->getRedirect2(index_in_second));
                if ( second->getNumberFree() == 0){
                    continue;
                }
            } else {
                file->setRedirect2(bitMap->Find(), index_in_second);
                DEBUG('R', "file have sector %d for indirection 2 at index %d\n", file->getRedirect2(index_in_second), index_in_second);
                second = new SecondIndirection();
            }
        }

        FirstIndirection *first_again = new FirstIndirection();
        int sector_tmp;
        if (second->getSector(second_indirection_index) != INVALID_SECTOR){
            sector_tmp = second->getSector(second_indirection_index);
            first_again->FetchFrom(sector_tmp);
            if ( first_again->getNumberFree() == 0){
                continue;
            }
        } else {
            sector_tmp = bitMap->Find();
            second->setSector(second_indirection_index  % MAX_INDIRECT_LEVEL_ONE, sector_tmp);
        }
        for (int first_indirection_index = MAX_INDIRECT_LEVEL_ONE - first_again->getNumberFree(); first_indirection_index < MAX_INDIRECT_LEVEL_ONE && *nb_necessary > 0; first_indirection_index++){
            first_again->setSector(first_indirection_index, bitMap->Find());
            (*nb_necessary)--;
        }
        first_again->WriteAt(sector_tmp);
    }
    // DEBUG('R', "file have sector %d saved for indirection 2 end of loop at index %d\n", file->getRedirect2(index_in_second), index_in_second);
    second->WriteAt(file->getRedirect2(index_in_second));
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return false if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------
bool FileHeader::Allocate(BitMap *bitMap, const int fileSize) {

    int nb_necessary = divRoundUp(fileSize, SectorSize) - numSectors;
    DEBUG('R', "Need %d sectors\n", numSectors);
    if (bitMap->NumClear() < nb_necessary) { // TODO count sector for redirection 
        DEBUG('R', "Need %d sectors but not enought space available\n", numSectors);
        return false; // not enough space
    }

    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);

    initializeDirectData(bitMap, &nb_necessary);
    if (nb_necessary <= 0){
        return true;
    }

    File * file = initializeFirstIndirection(bitMap, &nb_necessary);
    DEBUG('R', " remaining %d for second indirection \n", nb_necessary);
    if (nb_necessary <= 0){
        DEBUG('R', "C'est fini pour l'allocation\n");
        file->WriteBack(getRedirect());
        return true;
    }

    initializeSecondIndirection(bitMap, &nb_necessary, file);
    file->WriteBack(getRedirect());
    return true;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void FileHeader::Deallocate(BitMap *bitMap) const {
    int mini = MIN(numSectors, NumDirect);
    for (int i = 0; i < mini; i++) {
        if (dataSectors[i] == INVALID_SECTOR){
            continue;
        }
        ASSERT(bitMap->Test(dataSectors[i])); // ought to be marked!
        bitMap->Clear(dataSectors[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void FileHeader::FetchFrom(const int sectorNumber) {
    synchDisk->ReadSector(sectorNumber, reinterpret_cast<char *>(this));
    DEBUG('R', "après lecteure redircet = %d\n", redirect);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(const int sectorNumber) {
    synchDisk->WriteSector(sectorNumber, reinterpret_cast<char *>(this));
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

sector_t FileHeader::ByteToSector(const int offset) const {
    if (offset < 0){
        ASSERT(FALSE);
    }
    if ((unsigned int) offset / SectorSize < NumDirect ) {
        // DEBUG('R', "On renvoie %d dansByteToSector sans redirection car offset = %d\n", offset / SectorSize, offset);
        return dataSectors[offset / SectorSize];
    } else {
        File* file = new File();
        file->FetchFrom(this->redirect);
        int current = offset / SectorSize - NumDirect;
        FirstIndirection *first_indir = new FirstIndirection();
        first_indir->FetchFrom(file->getFirstIndirection());
        if (first_indir->InUse(current)){
            sector_t result = first_indir->getSector(current);
            // DEBUG('R', "On renvoie %d dansByteToSector cr offset =%d\n", result, offset);
            ASSERT(result != 0 );
            return result;
        }

        DEBUG('R', "Seconde inderction at sector %d\n", file->getRedirect2(0));
        SecondIndirection* file2 = new SecondIndirection();
        file2->FetchFrom(file->getRedirect2(0));
        current = offset / SectorSize - NumDirect - MAX_INDIRECT_LEVEL_ONE;
        int in_second = current / MAX_INDIRECT_LEVEL_ONE;
        int in_third = current % MAX_INDIRECT_LEVEL_ONE;
        if (file2->InUse(in_second)){
             FirstIndirection *third = new FirstIndirection();
             DEBUG('R', "on fetche le ecteur %d car il est à index %d\n", file2->getSector(in_second), in_second);
             third->FetchFrom(file2->getSector(in_second));

             DEBUG('R', "on fetche le ecteur %d car il est à index %d\n", third->getSector(in_third), in_third);
            sector_t result =third->getSector(in_third);
            DEBUG('R', "Seconde inderction : On renvoie %d dansByteToSector cr offset =%d\n", result, offset);
            ASSERT(result != 0 );
            return result;
        }


        ASSERT(FALSE);
        // if (DebugIsEnabled('R')){
        //     indirect->Print();
        // }
        return -1;
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int FileHeader::FileLength() const {
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void FileHeader::Print() const {
    int i;
    const auto data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    int mini = MIN(numSectors, NumDirect);
    for (i = 0; i < mini; i++) { printf("%d ", dataSectors[i]); }
    printf("\nFile contents:\n");
    for (int k = i = 0; i < mini; i++) {
        synchDisk->ReadSector(dataSectors[i], data);
        for (int j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
            if ('\040' <= data[j] && data[j] <= '\176') {   // isprint(data[j])
                // printf("%c", data[j]);
            } else {
                // printf("\\%x", static_cast<unsigned char>(data[j]));
            }
        }
        // printf("\n"); 
    }
    printf("------------------------------------------------------------------\n");
    delete [] data;
}

