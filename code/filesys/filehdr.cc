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

#include "fileindirection.h"
#include "system.h"
#include "filehdr.h"

#define MIN(a, b) \
    (int) a < (int) b ? a : b

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
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    DEBUG('R', "Need %d sectors\n", numSectors);
    if (bitMap->NumClear() < numSectors) {
        DEBUG('R', "Need %d sectors but not enought space available\n", numSectors);
        return false; // not enough space
    }

    int mini = MIN(numSectors, NumDirect);
    for (int i = 0; i < mini; i++) {
        dataSectors[i] = bitMap->Find();
    }
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

sector_t FileHeader::getRedirect(){return redirect;}

sector_t FileHeader::ByteToSector(const int offset) const {
    if (offset < 0){
        ASSERT(FALSE);
    }
    if ((unsigned int) offset / SectorSize < NumDirect ) {
        DEBUG('R', "On renvoie %d dansByteToSector sans redirection car offset = %d\n", offset / SectorSize, offset);
        return dataSectors[offset / SectorSize];
    } else {
        FirstIndirection* indirect = new FirstIndirection();
        indirect->FetchFrom(this->redirect);
        int current = offset / SectorSize - NumDirect;
        if (indirect->InUse(current)){
            sector_t result =indirect->getSector(current);
            DEBUG('R', "On renvoie %d dansByteToSector cr offset =%d\n", result, offset);
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

int FileHeader::WithoutIndirect() {
    return NumDirect;
}

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

