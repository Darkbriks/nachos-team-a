#ifndef FILECONST_H
#define FILECONST_H

#include "directoryentry.h"

#define NumDirect   ((SectorSize - 2 * sizeof(int)) / sizeof(int))
#define MaxFileSize (NumDirect * SectorSize)

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector   0
#define DirectorySector 1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize   (NumSectors / BitsInByte)
#define NumDirEntries     10
#define DirectoryFileSize (sizeof(DirectoryEntry) * NumDirEntries)

#define MAX_INODES 3

typedef int sector_t;
typedef int inode_t;

#endif // FILECONST_H
