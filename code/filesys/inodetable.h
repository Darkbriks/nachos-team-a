#ifndef INODETABLE_H
#define INODETABLE_H

#include "fileconst.h"

class BitMap;
class OpenFile;

class Inode{

    public:
        Inode():file(nullptr), sector(-1) {}
        Inode(OpenFile * f, int s): file(f), sector(s){}
        ~Inode();

        const OpenFile& getFile() const {return *file;}
        int getSector() const {return sector;}

    private:
        OpenFile* file;
        int sector;
};

class InodeTable{
    public:
        InodeTable();
        ~InodeTable();

        int Open(OpenFile * file, int sector);
        bool Close(int inode);
        Inode* getInode(int inode);
        void Print();

    private:
        Inode* inodes[MAX_INODES];
        BitMap* freeInodes;
};


#endif // INODETABLE_H
