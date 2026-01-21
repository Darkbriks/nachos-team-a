// directory.h 
//	Data structures to manage a UNIX-like directory of file names.
// 
//      A directory is a table of pairs: <file name, sector #>,
//	giving the name of each file in the directory, and 
//	where to find its file header (the data structure describing
//	where to find the file's data blocks) on disk.
//
//      We assume mutual exclusion is provided by the caller.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "copyright.h"
#include "fileconst.h"
#include "filetype.h"

class DirectoryEntry;
class OpenFile;

// The following class defines a UNIX-like "directory".  Each entry in
// the directory describes a file, and where to find it on disk.
//
// The directory data structure can be stored in memory, or on disk.
// When it is on disk, it is stored as a regular Nachos file.
//
// The constructor initializes a directory structure in memory; the
// FetchFrom/WriteBack operations shuffle the directory information
// from/to disk. 

class Directory {
public:
    explicit Directory(int size); // Initialize an empty directory with space for "size" files
    ~Directory(); // De-allocate the directory

    void FetchFrom(OpenFile *file) const; // Init directory contents from disk
    void WriteBack(OpenFile *file) const; // Write modifications to directory contents back to disk

    sector_t Find(const char *name) const; // Find the sector number of the FileHeader for file: "name"
    bool Add(const char *name, int newSector, File_Type type) const;  // Add a file name into the directory
    bool Remove(const char *name) const;	// Remove a file from the directory

    [[nodiscard]] unsigned int NbEntry() const;
    File_Type GetType(const char* name) const;
    [[nodiscard]] char* GetName(int index) const;
    [[nodiscard]] File_Type GetType(int index) const;
    [[nodiscard]] int GetSector(int index) const;

    void Tree(unsigned int tabulation = 3) const;
    void List() const; // Print the names of all the files in the directory
    void Print() const; // Verbose print of the contents of the directory -- all the file names and their contents.

    static Directory* getDirectory(int sector);

private:
    int tableSize; // Number of directory entries
    DirectoryEntry *table; // Table of pairs: <file name, file header location>

    int FindIndex(const char *name)const; // Find the index into the directory table corresponding to "name"
};

#endif // DIRECTORY_H
