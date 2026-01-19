// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "file.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"

Directory* Directory::getDirectory(const int sector) {
    auto* result = new Directory(NumDirEntries);
    OpenFile file(sector);
    result->FetchFrom(&file);
    return result;
}

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(const int size) {
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++) {
        table[i].inUse = false;
    }
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory() {
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void Directory::FetchFrom(OpenFile *file) const {
    (void) file->ReadAt(reinterpret_cast<char *>(table), tableSize * static_cast<int>(sizeof(DirectoryEntry)), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void Directory::WriteBack(OpenFile *file) const {
    (void) file->WriteAt(reinterpret_cast<char *>(table), tableSize * static_cast<int>(sizeof(DirectoryEntry)), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int Directory::FindIndex(const char *name) const {
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen)) {
            return i;
        }
    }
    return -1; // name not in directory
}

File_Type Directory::GetType(const char* name) const {
    if (const int i = FindIndex(name); i != -1) {
        return table[i].type;
    }
    return NULL_T;
}

char* Directory::GetName(const int index) const {
    if (index < 0 || index >= tableSize || !table[index].inUse) {
        return nullptr;
    }
    return table[index].name;
}

File_Type Directory::GetType(const int index) const {
    if (index < 0 || index >= tableSize || !table[index].inUse) {
        return NULL_T;
    }
    return table[index].type;
}

int Directory::GetSector(const int index) const {
    if (index < 0 || index >= tableSize || !table[index].inUse) {
        return -1;
    }
    return table[index].sector;
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int Directory::Find(const char *name) const {
    if (const int i = FindIndex(name); i != -1) {
        return table[i].sector;
    }
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return true if successful;
//	return false if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//	"type" -- the type of the document ( file, directory ... )
//----------------------------------------------------------------------

bool Directory::Add(const char *name, const int newSector, const File_Type type) const {
    DEBUG('f', "Try to add file %s in the directory \n", name);
    if (FindIndex(name) != -1) {
        DEBUG('f', "File %s already exist in this directory\n", name);
        return false;
    }

    for (int i = 0; i < tableSize; i++) {
        if (!table[i].inUse) {
            table[i].inUse = true;
            table[i].type = type;
            table[i].sector = newSector;
            strncpy(table[i].name, name, FileNameMaxLen); 
            DEBUG('f', "Sucessfully add file %s in the directory \n", name);
            return true;
        }
    }
    return false; // no space. Fix when we have extensible files.
}

/**
 * @brief return all entry in this directory including "." and ".."
 *
 * @return  The number of file and directory in this directory
 */
unsigned int Directory::NbEntry() const {
    unsigned int result = 0;
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse) {
            result++;
        }
    }
    return result;
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return true if successful;
//	return false if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool Directory::Remove(const char *name) const {
    const int i = FindIndex(name);

    if (i == -1) {
        DEBUG('f', "Don't found file %s to remove it\n", name);
        return false; // name not in directory
    }

    if (table[i].type == DIRECTORY_T) {
        DEBUG('f', "Try to remove directory %s\n", name);
        const Directory* child = getDirectory(table[i].sector);
        if (const unsigned int nb = child->NbEntry(); nb != 2) { // Two is for "." and ".."
            DEBUG('f', "Directory %s is not empty and contains %d iterms\n", name, nb);
            delete child;
            return false;
        }
        delete child;
    }

    DEBUG('f', "Successfully deleted file %s\n", name);
    table[i].inUse = false;
    return true;
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void Directory::List() const {
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse) {
            FileHeader *file = new FileHeader();
            file->FetchFrom(table[i].sector);
            printf("%c %s %d bits\n", file_type_to_char(table[i].type), table[i].name, file->FileLength());
        }
    }
}

#define TAB(n) for (unsigned int xyz = 0; xyz < n; xyz++) { printf(" "); }

void Directory::Tree(const unsigned int tabulation) const {
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse) {
            TAB(tabulation);
            printf("%c %s\n", file_type_to_char(table[i].type), table[i].name);
            if (table[i].type == DIRECTORY_T && strcmp(table[i].name, ".") != 0 && strcmp(table[i].name, "..") != 0) {
                const Directory* subdir = getDirectory(table[i].sector);
                subdir->Tree(tabulation + 3);
                delete subdir;
            }
        }
    }
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void Directory::Print() const {
    auto *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse) {
            printf("------------------------------------------------------------------\n");
            printf("Name: %s, Sector: %d de type %s\n", table[i].name, table[i].sector, file_type_to_str(table[i].type));
            hdr->FetchFrom(table[i].sector);
            if (hdr->getRedirect() != -1 && table[i].type == FILE_T) {
                File *file = new File();
                file->FetchFrom(hdr->getRedirect());
                file->Print();
            }
            hdr->Print();
        }
    }
    printf("\n");
    delete hdr;
}
