// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "file.h"
#include "fileindirection.h"
#include "filehdr.h"
#include "system.h"
#include "filesys.h"
#include "inodetable.h"
#include "pathnavigator.h"

extern void Print(const char *file);

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = true, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = false, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(const bool format) {
    DEBUG('f', "Initializing the file system.\n");
    inodes.Open(DirectorySector);
    if ( ! format) {
        // if we are not formatting the disk, just open the files representing
        // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
        return;
    }

    auto *freeMap = new BitMap(NumSectors);
    const auto *directory = new Directory(NumDirEntries);
    auto *mapHdr = new FileHeader;
    auto *dirHdr = new FileHeader;

    DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
    freeMap->Mark(FreeMapSector);	    
    freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

    ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
    ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

    DEBUG('f', "Writing headers back to disk.\n");
    mapHdr->WriteBack(FreeMapSector);    
    dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

    freeMapFile = new OpenFile(FreeMapSector);
    directoryFile = new OpenFile(DirectorySector);

    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

    DEBUG('f', "Writing bitmap and directory back to disk.\n");
    freeMap->WriteBack(freeMapFile);	 // flush changes to disk
    directory->WriteBack(directoryFile);
    createSubDirectory(1, 1, dirHdr, freeMap);

    if (DebugIsEnabled('f')) {
        freeMap->Print();
        directory->Print();

    }
    delete freeMap; 
    delete directory; 
    delete mapHdr; 
    delete dirHdr;
}

bool FileSystem::Create(const char* name, const int initialSize, const File_Type type) {
    const PathNavigator nav(this, name);
    if (!nav.isValid()) { return false; }
    return _Create(nav.getLastComponent(), initialSize, type);
}

bool FileSystem::Remove(const char *name) {
    const PathNavigator nav(this, name);
    if (!nav.isValid()) { return false; }
    return _Remove(nav.getLastComponent());
}

OpenFile* FileSystem::Open(const char *name) {
    const PathNavigator nav(this, name);
    if (!nav.isValid()) { return nullptr; }
    return inodes.GetFile(_Open(nav.getLastComponent()));
}

bool FileSystem::Close(const char *name) {
    const PathNavigator nav(this, name);
    if (!nav.isValid()) { return false; }
    return _Close(nav.getLastComponent());
}

bool FileSystem::Change_Directory(const char * name) {
    const PathNavigator nav(this, name, false);
    if (!nav.isValid()) { return false; }
    return _Change_Directory(nav.getLastComponent());
}

void FileSystem::ReadAllFile(const char* name) {
    const PathNavigator nav(this, name);
    if (!nav.isValid()) { return; }
    Print(nav.getLastComponent());
}

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void FileSystem::List() const {
    const auto *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void FileSystem::Print_FS() const {
    auto *bitHdr = new FileHeader;
    auto *dirHdr = new FileHeader;
    auto *freeMap = new BitMap(NumSectors);
    const auto *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    printf("BITMAP !!!!!!!!!!!!!!!!!\n");
    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();


    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
}

int FileSystem::GetWorkingSector() const {
    return directoryFile->GetSector();
}

char* FileSystem::GetWorkingPath() const {
    auto* path = new char[1024];
    path[0] = '\0';
    int currentSector = directoryFile->GetSector();

    while (currentSector != DirectorySector) {
        const auto* directory = new Directory(NumDirEntries);
        OpenFile currentFile(currentSector);
        directory->FetchFrom(&currentFile);
        const int parentSector = directory->Find("..");

        const auto* parentDirectory = new Directory(NumDirEntries);
        OpenFile parentFile(parentSector);
        parentDirectory->FetchFrom(&parentFile);

        for (int i = 0; i < NumDirEntries; i++) {
            if (parentDirectory->GetType(i) == DIRECTORY_T &&
                parentDirectory->GetSector(i) == currentSector) {
                char temp[1024];
                snprintf(temp, sizeof(temp), "/%s%s", parentDirectory->GetName(i), path);
                strncpy(path, temp, 1024);
                break;
            }
        }
        currentSector = parentSector;
        delete directory;
        delete parentDirectory;
    }

    if (path[0] == '\0') { strncpy(path, "/", 1024); }
    return path;
}

void FileSystem::Tree() const {
    printf("d /\n");
    const auto *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    directory->Tree();
    delete directory;
}

void FileSystem::DisplayInodes() {
    inodes.Print();
}

/**
 * Create the entries '.' and '..' in the new created subdirectory
 * @param prev_sector The sector of the parent directory
 * @param curr_sector The sector of the new created subdirectory
 * @param hdr The file header of the new created subdirectory
 * @param freeMap The bitmap of free sectors
 * @return true if the operation is successful, false otherwise
 */
bool FileSystem::createSubDirectory(const int prev_sector, const int curr_sector, FileHeader* hdr, BitMap *freeMap) const {
    const Directory *directoryChild = Directory::getDirectory(curr_sector);
    DEBUG('f', "Try to create '.'\n");

    if (!directoryChild->Add(".", curr_sector, DIRECTORY_T)) {
        DEBUG('f', "Failed to create '.', sector %d\n", curr_sector);
        delete directoryChild;
        return false;
    }
    DEBUG('f', "Sucessfully created '.'\n");

    DEBUG('f', "Try to create '..'\n");
    if (!directoryChild->Add("..", prev_sector, DIRECTORY_T)) {
        DEBUG('f', "Failed to create '..', sector %d\n", prev_sector);
        delete directoryChild;
        return false;
    }
    DEBUG('f', "Sucessfully created '..'\n");

    DEBUG('f', "Write back subdirectory header and content to disk\n");
    hdr->WriteBack(curr_sector);

    OpenFile file(curr_sector);
    directoryChild->WriteBack(&file);
    freeMap->WriteBack(freeMapFile);

    delete directoryChild;
    return true;
}

    void FileSystem::SetCurrentDirectory(sector_t sector) {
        ASSERT(sector != -1);
        int prev_inode = inodes.FindBySector(directoryFile->GetSector());
        if (directoryFile->GetSector() == sector){
            return;
        }
        if (prev_inode != -1) { inodes.Close(prev_inode); }
        int inode = inodes.Open(sector);
        ASSERT(inodes.GetFile(inode) != nullptr);
        directoryFile = inodes.GetFile(inode);
    }
//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//    Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return true if everything goes ok, otherwise, return false.
//
// 	Create fails if:
//   	file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

void FileSystem::createFile(int initialSize, BitMap* freeMap, FileHeader* hdr){
    if ((unsigned int) initialSize < NumDirect * SectorSize){
        DEBUG('R', "Don't need indirection sector\n");
        hdr->setRedirect(-1);
    } else{
        const sector_t redirect = freeMap->Find();
        DEBUG('f', "!!!!!!!!!!!!!!!!!!  file have sector %d for indirection\n", redirect);
        hdr->setRedirect(redirect);
        File  * first = new File();
        int nb_necessary = divRoundUp(initialSize - NumDirect * SectorSize, SectorSize);

        DEBUG('R', "file have nb = %d sector car déjà fais = %d\n", nb_necessary, NumDirect * SectorSize);
        first->indirect.nbEntry = nb_necessary + 1;


        for (int i = 0; i < nb_necessary && i < MAX_INDIRECT_LEVEL_ONE; i++){
            int sector_tmp = freeMap->Find();
            ASSERT(sector_tmp != FreeMapSector);
            first->indirect.entries[i].setSector(sector_tmp);
            first->indirect.entries[i].setUse(true);
        }
        nb_necessary -= MAX_INDIRECT_LEVEL_ONE;
        if (nb_necessary > 0){
            // TODO faire autant d'indirection niveau 2 que nécessaire  !!!
        }

        first->WriteBack(redirect);
        hdr->setRedirect(redirect);
    }
}

#define CreateCleanup(success) \
    delete hdr; \
    delete freeMap; \
    delete directory; \
    return success;

bool FileSystem::_Create(const char *name, const int initialSize, const File_Type type) const {
    DEBUG('f', "Creating %s %s, size %d\n", file_type_to_str(type), name, initialSize);

    const Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1) {
        DEBUG('f', "Creating %s %s impossible, it lready exists\n", file_type_to_str(type), name);
        delete directory;
        return false;
    }

    auto *hdr = new FileHeader;
    auto *freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);
    const int sector = freeMap->Find();

    if (sector == -1) {
        DEBUG('f', "Creating %s %s impossible, no free block for file header\n", file_type_to_str(type), name);
        CreateCleanup(false);
    }

    if (!directory->Add(name, sector, type)) {
        DEBUG('f', "Creating %s %s impossible, no space in directory\n", file_type_to_str(type), name);
        CreateCleanup(false);
    }

    if (!hdr->Allocate(freeMap, initialSize )){
        DEBUG('f', "Creating %s %s impossible, no space on disk for data\n", file_type_to_str(type), name);
        CreateCleanup(false);
    }

    // Set all bytes to zero
    // This resolves strange behavior observed when DISK file isn't deleted, but "-f" flag is used
    const auto zeroes = new char[initialSize];
    bzero(zeroes, initialSize);
    OpenFile tempFile(sector);
    tempFile.Write(zeroes, initialSize);
    delete[] zeroes;

    if (type == DIRECTORY_T && !createSubDirectory(directoryFile->GetSector(), sector, hdr, freeMap)) {
        DEBUG('f', "Creating %s %s impossible, can't create subdirectory\n", file_type_to_str(type), name);
        CreateCleanup(false);
    } else if ( type == FILE_T ) {
        createFile(initialSize, freeMap, hdr);
        
    }
    ASSERT(FreeMapSector == 0);

    // everthing worked, flush all changes back to disk
    hdr->WriteBack(sector);
    directory->WriteBack(directoryFile);
    freeMap->WriteBack(freeMapFile);
    DEBUG('f', "Sucessfully created %s %s on disk sector = %d\n", file_type_to_str(type), name, sector);
    CreateCleanup(true);
}

sector_t FileSystem::GetSectorByName(const char* name) {
    const auto *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    const int result = directory->Find(name);
    delete directory;
    return result;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

inode_t FileSystem::_Open(const char *name) {
    const sector_t sector = GetSectorByName(name);
    if (sector < 0) {
        DEBUG('f', "Don't find file %s\n", name);
        return -1;
    }

    const inode_t inode = inodes.Open(sector);
    if (inode < 0) {
        DEBUG('f', "No more space in inodes table\n");
        return -1;
    }

    DEBUG('f', "File %s is open\n", name);
    return inode;
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return true if the file was deleted, false if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool FileSystem::_Remove(const char *name) {
    DEBUG('f', "Try to delete file %s\n", name);

    const sector_t sector = GetSectorByName(name);

    if (sector == -1) {
        DEBUG('f', "File %s is not found\n", name);
        return false; // file not found
    }

    const int inodeRefCount = inodes.GetRefCount(inodes.FindBySector(sector));
    if (inodeRefCount > 0) {
        DEBUG('f', "Can't delete file %s, it is still opened\n", name);
        return false; // file is still opened
    }

    const auto *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    if (!directory->Remove(name)) {
        delete directory;
        DEBUG('f', "Can't delete file %s\n", name);
        return false;
    }

    auto *fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    auto *freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap); // remove data blocks
    freeMap->Clear(sector);       // remove header block

    freeMap->WriteBack(freeMapFile);     // flush to disk
    directory->WriteBack(directoryFile); // flush to disk
    DEBUG('f', "File %s is deleted\n", name);
    delete fileHdr;
    delete directory;
    delete freeMap;
    return true;
}

bool FileSystem::_Close(const char* name){
    DEBUG('f', "Try to close file %s\n", name);

    const sector_t sector = GetSectorByName(name);

    if (sector == -1) {
        DEBUG('f', "File %s is not found\n", name);
        return false; // file not found
    }

    const int inodeRefCount = inodes.Close(inodes.FindBySector(sector));
    if (inodeRefCount == -1) {
        DEBUG('f', "Error closing inode for file %s\n", name);
        return false;
    }

    DEBUG('f', "Successfuly closing inode for file %s\n", name);
    return true;
}

bool FileSystem::_Change_Directory(const char * name) {

    const auto *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    if (directory->GetType(name) != DIRECTORY_T) {
        DEBUG('f', "Directory %s can't be the active directory because it doesn't exist\n", name);
        delete directory;
        return false;
    }
    delete directory;

    SetCurrentDirectory(GetSectorByName(name));
    DEBUG('f', "Change directory now in %s at sector %d \n", name, directoryFile->GetSector());

    return true;
}
