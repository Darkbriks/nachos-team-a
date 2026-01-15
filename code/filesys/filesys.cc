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
#include "filehdr.h"
#include "system.h"
#include "filesys.h"

extern void Print(const char *file);

#define RESTORE_void(cond)                               \
    if (! cond){                                    \
        directoryFile = __BACKUP_RESTORE__;         \
        for (int xyz = 0; xyz < MAX_PATH_SIZE; xyz++){    \
            free(real_path[xyz]);                     \
        }                                           \
        free(real_path);                            \
        return ;                               \
    } 

#define RESTORE(cond, result)                               \
    if (! cond){                                    \
        directoryFile = __BACKUP_RESTORE__;         \
        for (int xyz = 0; xyz < MAX_PATH_SIZE; xyz++){    \
            free(real_path[xyz]);                     \
        }                                           \
        free(real_path);                            \
        return result;                               \
    } 

#define SAVE()                                      \
    OpenFile * __BACKUP_RESTORE__ = directoryFile;  
/*
 * You must declare :
 * char * lastDir
 * and you command will be executed on this char * which is the last name after the last /
 * Don't put , in the command, only one instruction is accepted  !!!!
 *
 * You must return a bool
 */
#define COMPUTE_PATH_AND_EXECUTE_void(name, goBack, line)                            \
    SAVE();                                                                          \
    char ** real_path = (char **) calloc(sizeof(char *) * MAX_PATH_SIZE, 1);         \
    for (int i = 0; i < MAX_PATH_SIZE; i++){                                         \
        real_path[i] = (char *) calloc(sizeof(char) * MAX_PATH_SIZE, 1);             \
    }                                                                                \
    int size = parseName(name, real_path);                                           \
    for (int i = 0; i < size - 1; i++){                                              \
        DEBUG('f', "On cd sur %s depuis la commande %s\n", real_path[i], name);      \
        RESTORE_void(_Change_Directory(real_path[i]));                             \
    }                                                                                \
    lastDir = real_path[size - 1];                                                   \
    do{ line } while(0);                                                    \
    if (goBack){ RESTORE_void(false); }                                           \
    for (int i = 0; i < MAX_PATH_SIZE; i++){  free(real_path[i]); }                  \
    free(real_path);                                                                 \
    return;     



/*
 * You must declare :
 * char * lastDir
 * and you command will be executed on this char * which is the last name after the last /
 * Don't put , in the command, only one instruction is accepted  !!!!
 *
 * You must return a bool
 */
#define COMPUTE_PATH_AND_EXECUTE_bool(name, goBack, line)                                 \
    SAVE();                                                                          \
    char ** real_path = (char **) calloc(sizeof(char *) * MAX_PATH_SIZE, 1);         \
    for (int i = 0; i < MAX_PATH_SIZE; i++){                                         \
        real_path[i] = (char *) calloc(sizeof(char) * MAX_PATH_SIZE, 1);             \
    }                                                                                \
    int size = parseName(name, real_path);                                           \
    for (int i = 0; i < size - 1; i++){                                              \
        DEBUG('f', "On cd sur %s depuis la commande %s\n", real_path[i], name);      \
        RESTORE(_Change_Directory(real_path[i]), false);                             \
    }                                                                                \
    lastDir = real_path[size - 1];                                                   \
    bool result;                                                               \
    do{ result = line } while(0);                                                    \
    if (goBack){ RESTORE(false, result); }                                           \
    for (int i = 0; i < MAX_PATH_SIZE; i++){  free(real_path[i]); }                  \
    free(real_path);                                                                 \
    return result;     

/*
 * You must declare :
 * char * lastDir
 * and you command will be executed on this char * which is the last name after the last /
 * Don't put , in the command, only one instruction is accepted  !!!!
 *
 * You must return a bool
 */
#define COMPUTE_PATH_AND_EXECUTE_Openfile_ptr(name, goBack, line)                                 \
    SAVE();                                                                          \
    char ** real_path = (char **) calloc(sizeof(char *) * MAX_PATH_SIZE, 1);         \
    for (int i = 0; i < MAX_PATH_SIZE; i++){                                         \
        real_path[i] = (char *) calloc(sizeof(char) * MAX_PATH_SIZE, 1);             \
    }                                                                                \
    int size = parseName(name, real_path);                                           \
    for (int i = 0; i < size - 1; i++){                                              \
        DEBUG('f', "On cd sur %s depuis la commande %s\n", real_path[i], name);      \
        RESTORE(_Change_Directory(real_path[i]), nullptr);                             \
    }                                                                                \
    lastDir = real_path[size - 1];                                                   \
    OpenFile *result;                                                               \
    do{ result = line } while(0);                                                    \
    if (goBack){ RESTORE(false, result); }                                           \
    for (int i = 0; i < MAX_PATH_SIZE; i++){  free(real_path[i]); }                  \
    free(real_path);                                                                 \
    return result;     


//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format){
    DEBUG('f', "Initializing the file system.\n");
    freeInodes = new BitMap(MAX_INODES);
    if ( ! format) {
        // if we are not formatting the disk, just open the files representing
        // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
        return;
    }
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);
    FileHeader *mapHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;

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

    if (DebugIsEnabled('f')) {
        freeMap->Print();
        directory->Print();

        delete freeMap; 
        delete directory; 
        delete mapHdr; 
        delete dirHdr;
    }
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
//	Return TRUE if everything goes ok, otherwise, return FALSE.
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

bool FileSystem::_Create(const char *name, int initialSize, File_Type type){
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success = TRUE;

    DEBUG('f', "Creating %s %s, size %d\n", file_type_to_str(type), name, initialSize);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1) {
        success = FALSE;			// file is already in directory
        DEBUG('f', "Creating %s %s impossible, it lready exists\n", file_type_to_str(type), name);
        delete directory;
        return success;
    } 
    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);
    sector = freeMap->Find();	// find a sector to hold the file header
    if (sector == -1) { 		
        success = FALSE;		// no free block for file header 
    } else if (!directory->Add(name, sector, type)) {
        success = FALSE;	// no space in directory
    } else {
        hdr = new FileHeader;
        if (!hdr->Allocate(freeMap, initialSize)){
            success = FALSE;	// no space on disk for data
        } else {	
            // everthing worked, flush all changes back to disk
            if (type == DIRECTORY_T){
                if ( ! FileSystem::createSubDirectory(name, directoryFile->GetSector(), sector, hdr, freeMap) ){
                    success = FALSE;
                }
            }
            if (success){
                hdr->WriteBack(sector); 		
                directory->WriteBack(directoryFile);
                freeMap->WriteBack(freeMapFile);
                DEBUG('f', "Sucessfully created %s on disk sector = %d\n", name, sector);
            }
        }
        delete hdr;
    }
    delete freeMap;
    delete directory;
    return success;
}

bool FileSystem::createSubDirectory(const char* name, int sector_parent, int sector_directory_child, FileHeader* hdr, BitMap *freeMap){
    Directory *directoryChild = Directory::getDirectory(sector_directory_child);
    DEBUG('f', "Try to create '.'\n");
    if ( ! directoryChild->Add(".", sector_directory_child, DIRECTORY_T)){
        DEBUG('f', "Echec car sector renvoyé = %d en ajoutant au directory sur le secteur %d\n", sector_directory_child, sector_directory_child);

        return FALSE;
    }
    DEBUG('f', "Sucessfully created '.'\n");
    DEBUG('f', "Try to create '..'\n");
    if ( ! directoryChild->Add("..", sector_parent, DIRECTORY_T)) {
        return FALSE;
    }
    DEBUG('f', "Sucessfully created '..'\n");
    hdr->WriteBack(sector_directory_child); 		
    directoryChild->WriteBack(new OpenFile(sector_directory_child));
    freeMap->WriteBack(freeMapFile);
    DEBUG('f', "Sucessfully created '..' and '.' on disk on sector %d and %d\n", sector_parent, sector_directory_child);
    delete directoryChild;
    return TRUE;
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

OpenFile* FileSystem::_Open(const char *name){
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    DEBUG('f', "Opening file %s\n", name);
    if (sector >= 0) {
        openFile = new OpenFile(sector);	// name was found in directory 
    } else {
        DEBUG('f', "Don't find file %s\n", name);
    }
    delete directory;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool FileSystem::_Remove(const char *name){
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;

    DEBUG('f', "Try to delete file %s\n", name);
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
        delete directory;
        DEBUG('f', "File %s is not found\n", name);
        return FALSE;			 // file not found 
    }
    if ( ! directory->Remove(name)){
        DEBUG('f', "Can't delete file %s\n", name);
        return FALSE;
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    DEBUG('f', "File %s is deleted\n", name);
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void FileSystem::List(){
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

void FileSystem::Tree(){
    printf("d /\n");
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->Tree();
}

void FileSystem::PrintWorkingDirectory(){
    printf("Working directory = %d\n",directoryFile->GetSector());
}

int FileSystem::parseName(const char *name, char** result){
    bool isEchapped = false;
    int posInWord = 0;
    int wordInResult = 0;
    for (int i = 0; name[i] != 0; i++){
        if (name[i] == '\\' ){
            isEchapped = ! isEchapped;
        } else if (name[i] == '/'){
            if (!isEchapped){
                wordInResult++;
                posInWord = 0;
                continue;
            }
        }
        if ( ! isEchapped){
            result[wordInResult][posInWord] = name[i];
            posInWord++;
            isEchapped = false;
        }
    }
    result[wordInResult + 1] = nullptr;
    return wordInResult + 1;
}

bool FileSystem::_Change_Directory(const char * name){
    OpenFile * file;
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    if (directory->GetType(name) != DIRECTORY_T){
        DEBUG('f', "Directory %s can't be the active directory because it doesn't exist\n", name);
        return false;
    }
    if ( ( file = Open(name) ) == nullptr){
        ASSERT(FALSE);
        return false;
    }
    directoryFile = file;
    DEBUG('f', "Change directory now in %s at sector %d \n", name, directoryFile->GetSector());
    return true;
}

bool FileSystem::Change_Directory(const char * name){
    char *lastDir;
    COMPUTE_PATH_AND_EXECUTE_bool(name, false, _Change_Directory(lastDir););
}

OpenFile* FileSystem::Open(const char *name){
    char *lastDir;
    COMPUTE_PATH_AND_EXECUTE_Openfile_ptr(name, true, _Open(lastDir););
}

void FileSystem::ReadAllFile(const char* name){
    char *lastDir;
    COMPUTE_PATH_AND_EXECUTE_void(name, true, Print(lastDir););
}

bool FileSystem::Remove(const char *name){
    char *lastDir;
    COMPUTE_PATH_AND_EXECUTE_bool(name, true, _Remove(lastDir););
}

bool FileSystem::Create(const char *name, int initialSize, File_Type type){
    char *lastDir;
    COMPUTE_PATH_AND_EXECUTE_bool(name, true, _Create(lastDir, initialSize, type););
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

void FileSystem::Print_FS(){
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 
