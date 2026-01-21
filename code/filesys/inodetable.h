#ifndef INODETABLE_H
#define INODETABLE_H

#include "fileconst.h"
#include "utility.h"

class BitMap;
class OpenFile;

class Inode {
public:
    Inode() : file(nullptr), sector(-1) {}
    Inode(OpenFile* f, const int s) : file(f), sector(s) {}
    ~Inode();

    [[nodiscard]] OpenFile& getFile() const { return *file; }
    [[nodiscard]] OpenFile* getFilePtr() { return file; }
    [[nodiscard]] int getSector() const { return sector; }
    [[nodiscard]] int getRefCount() const { return refCount; }
    void incrementRefCount() { refCount++; }
    void decrementRefCount() { if (refCount > 0) refCount--; else ASSERT(FALSE); }

private:
    OpenFile* file;
    sector_t sector;
    int refCount = 0;
};

class InodeTable {
public:
    InodeTable();
    ~InodeTable();

    inode_t Open(sector_t sector);
    int Close(inode_t inode); // returns new ref count (0 if closed), -1 on error
    int CloseBySector(sector_t sector); // returns new ref count (0 if closed), -1 on error

    [[nodiscard]] bool IsOpen(inode_t inode) const;
    [[nodiscard]] inode_t FindBySector(sector_t sector) const;
    [[nodiscard]] inode_t FindByFile(OpenFile* file);

    [[nodiscard]] OpenFile* GetFile(inode_t inode) const;
    [[nodiscard]] int GetRefCount(inode_t inode) const;
    [[nodiscard]] sector_t GetSector(inode_t inode) const;

    void Print()const;

private:
    Inode* inodes[MAX_INODES] {};
    BitMap* freeInodes;
};


#endif // INODETABLE_H
