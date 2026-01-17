// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"
#include "filetype.h"
#include "fileconst.h"
#include "inodetable.h"

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
                            // calls to UNIX, until the real file system
                            // implementation is available
class FileSystem {
    public:
        FileSystem(bool format) {}

        bool Create(const char *name, int initialSize) { 
            int fileDescriptor = OpenForWrite(name);

            if (fileDescriptor == -1) return FALSE;
            Close(fileDescriptor); 
            return TRUE; 
        }

        OpenFile* Open(char *name) {
            int fileDescriptor = OpenForReadWrite(name, FALSE);

            if (fileDescriptor == -1) return NULL;
            return new OpenFile(fileDescriptor);
        }

        bool Remove(char *name) { return Unlink(name) == 0; }

};

#else // FILESYS

class Directory;
class InodeTable;


class FileSystem {
    friend class PathNavigator;
public:
    explicit FileSystem(bool format); // Initialize the file system.
                                      // Must be called *after* "synchDisk"
                                      // has been initialized.
                                      // If "format", there is nothing on
                                      // the disk, so initialize the directory
                                      // and the bitmap of free blocks.
    ~FileSystem() = default;

    bool Create(const char *name, int initialSize, File_Type type = FILE_T); // Create a file (UNIX creat)
    OpenFile* Open(const char *name); // Open a file (UNIX open)
    bool Close(const char* name);
    bool Remove(const char *name); // Delete a file (UNIX unlink)

    bool Change_Directory(const char * name);
    void ReadAllFile(const char* name);

    void List()const;     // List all the files in the file system
    void Print_FS()const; // List all the files and their contents
    [[nodiscard]] int GetWorkingSector() const;
    [[nodiscard]] char* GetWorkingPath() const;

    void Tree()const;

    void DisplayInodes();


    [[nodiscard]] OpenFile* GetCurrentDirectory() const { return directoryFile; }
    void SetCurrentDirectory(sector_t sector);

private:
    InodeTable inodes;
    OpenFile* freeMapFile;		// Bit map of free disk blocks,
                                    // represented as a file
    OpenFile* directoryFile;		// "Root" directory -- list of
                                    // file names, represented as a file
    bool createSubDirectory(int prev_sector, int curr_sector, FileHeader* hdr, BitMap *freeMap)const;
    static void createFile(int initialSize, BitMap* freeMap, FileHeader* hdr);

    sector_t GetSectorByName(const char* name);
    bool _Create(const char *name, int initialSize, File_Type type) const;
    inode_t _Open(const char *name); 	// Open a file (UNIX open)
    bool _Close(const char* name);
    bool _Remove(const char *name);
    bool _Change_Directory(const char * name);
};

#endif // FILESYS

#endif // FS_H
