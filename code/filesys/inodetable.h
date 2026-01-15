#ifndef INODETABLE_H
#define INODETABLE_H

#include "fileconst.h"

class BitMap;
class OpenFile;

class Inode {
public:
    Inode():file(nullptr), sector(-1) {}
    Inode(OpenFile * f, const int s): file(f), sector(s){}
    ~Inode();

    [[nodiscard]] const OpenFile& getFile() const { return *file; }
    [[nodiscard]] int getSector() const { return sector; }

private:
    OpenFile* file;
    int sector;
};

class InodeTable {
public:
    InodeTable();
    ~InodeTable();

    int Open(OpenFile * file, int sector);
    bool Close(int inode);
    [[nodiscard]] Inode* getInode(int inode) const;
    void Print()const;

private:
    Inode* inodes[MAX_INODES] {};
    BitMap* freeInodes;
};


#endif // INODETABLE_H
