#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H
#include "filetype.h"

#define FileNameMaxLen 9 // for simplicity, we assume file names are <= 9 characters long

// The following class defines a "directory entry", representing a file
// in the directory.  Each entry gives the name of the file, and where
// the file's header is to be found on disk.
//
// Internal data structures kept public so that Directory operations can
// access them directly.
class DirectoryEntry {
public:
    bool inUse;				// Is this directory entry in use?
    int sector;				// Location on disk to find the FileHeader for this file
    File_Type type;         // The type of file (file, directory, ...)
    char name[FileNameMaxLen + 1];	// Text name for file, with +1 for the trailing '\0'
};

#endif // DIRECTORYENTRY_H
