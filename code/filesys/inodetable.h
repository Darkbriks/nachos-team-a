#ifndef INODETABLE_H
#define INODETABLE_H

#include "fileconst.h"

class BitMap;
class OpenFile;

class Inode {
public:
    Inode() : file(nullptr), sector(-1) {}
    Inode(OpenFile* f, const int s) : file(f), sector(s) {}
    ~Inode();

    [[nodiscard]] OpenFile& getFile() const { return *file; }
    [[nodiscard]] int getSector() const { return sector; }
    [[nodiscard]] int getRefCount() const { return refCount; }
    void incrementRefCount() { refCount++; }
    void decrementRefCount() { if (refCount > 0) refCount--; }

private:
    OpenFile* file;
    int sector;
    int refCount = 0;
};

class InodeTable {
public:
    InodeTable();
    ~InodeTable();

    int Open(int sector);
    int Close(int inode); // returns new ref count (0 if closed), -1 on error
    int CloseBySector(int sector); // returns new ref count (0 if closed), -1 on error

    [[nodiscard]] bool IsOpen(int inode) const;
    [[nodiscard]] int FindBySector(int sector) const;

    [[nodiscard]] OpenFile* GetFile(int inode) const;
    [[nodiscard]] int GetRefCount(int inode) const;

    void Print()const;

private:
    Inode* inodes[MAX_INODES] {};
    BitMap* freeInodes;
};


#endif // INODETABLE_H
