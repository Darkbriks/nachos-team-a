#include "inodetable.h"
#include "bitmap.h"

Inode::~Inode() {
    delete file;
}

InodeTable::InodeTable() {
    freeInodes = new BitMap(MAX_INODES);
    DEBUG('I', "Inode table initialized with size %d\n", MAX_INODES);
}

InodeTable::~InodeTable() {
    DEBUG('I', "Delete Inode table \n");
    delete freeInodes;
}

int InodeTable::Open(OpenFile * file, const int sector) {
    DEBUG('I', "Inode table : Try to add file %p with sector %d\n", file, sector);
    const int result = freeInodes->Find();
    if (result < 0){ 
        DEBUG('I', "Inode table : But it fails\n");
        return result;
    }
    DEBUG('I', "Inode table : And it works\n");
    inodes[result] = new Inode(file, sector);
    return result;
}

bool InodeTable::Close(const int inode) {
    DEBUG('I', "Inode table : Try to remove indoe %d\n", inode);
    if ( ! freeInodes->Test(inode)){
        DEBUG('I', "Inode table : But it fails\n");
        return false;
    }
    DEBUG('I', "Inode table : And it works\n");
    delete inodes[inode];
    inodes[inode] = nullptr;
    freeInodes->Clear(inode);
    return true;
}

Inode* InodeTable::getInode(const int inode) const {
    DEBUG('I', "Inode table : Try to fetch inode %d\n", inode);
    if ( ! freeInodes->Test(inode)){
        DEBUG('I', "Inode table : But it fails\n");
        return nullptr;
    }
    DEBUG('I', "Inode table : And it works\n");
    return inodes[inode];
}

void InodeTable::Print() const {
    for (int i = 0; i < MAX_INODES; i++){
        if (inodes[i]){
            printf("At inode %d we store sector %d\n", i, inodes[i]->getSector());
        } else{
            printf("At inode %d we store nothing ie NIL\n", i);
        }

    }
}
