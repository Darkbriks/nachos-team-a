#include "inodetable.h"
#include "bitmap.h"

Inode::~Inode() {
    delete file;
}

InodeTable::InodeTable() : freeInodes(new BitMap(MAX_INODES)) {
    DEBUG('I', "Inode table initialized with size %d\n", MAX_INODES);
}

InodeTable::~InodeTable() {
    DEBUG('I', "Delete Inode table \n");
    delete freeInodes;
}

inode_t InodeTable::Open(const sector_t sector) {
    // Check if file is already opened
    int inodeIndex = FindBySector(sector);
    if (inodeIndex != -1) {
        DEBUG('I', "Inode for sector %d already opened at index %d\n", sector, inodeIndex);
        inodes[inodeIndex]->incrementRefCount();
        return inodeIndex;
    }

    // Find a free inode slot
    inodeIndex = freeInodes->Find();
    if (inodeIndex == -1) {
        DEBUG('I', "No free inode slots available\n");
        return -1;
    }

    // Create a new inode
    auto* file = new OpenFile(sector);
    inodes[inodeIndex] = new Inode(file, sector);
    inodes[inodeIndex]->incrementRefCount();
    DEBUG('I', "Opened new inode for sector %d at index %d\n", sector, inodeIndex);
    return inodeIndex;
}

int InodeTable::Close(const inode_t inode) {
    if (inode < 0 || inode >= MAX_INODES || inodes[inode] == nullptr) {
        DEBUG('I', "Invalid inode index %d for close\n", inode);
        return -1;
    }

    inodes[inode]->decrementRefCount();
    DEBUG('I', "Decremented ref count for inode %d, new ref count: %d\n", inode, inodes[inode]->getRefCount());

    if (inodes[inode]->getRefCount() == 0) {
        delete inodes[inode];
        inodes[inode] = nullptr;
        freeInodes->Clear(inode);
        DEBUG('I', "Closed inode %d and freed slot\n", inode);
        return 0;
    }

    return inodes[inode]->getRefCount();
}

int InodeTable::CloseBySector(const sector_t sector) {
    const int inodeIndex = FindBySector(sector);
    if (inodeIndex == -1) {
        DEBUG('I', "No open inode found for sector %d to close\n", sector);
        return -1;
    }
    return Close(inodeIndex);
}

bool InodeTable::IsOpen(const inode_t inode) const {
    return inode >= 0 && inode < MAX_INODES && inodes[inode] != nullptr;
}

inode_t InodeTable::FindBySector(const sector_t sector) const {
    for (int i = 0; i < MAX_INODES; ++i) {
        if (inodes[i] != nullptr && inodes[i]->getSector() == sector) {
            return i;
        }
    }
    return -1;
}

inode_t InodeTable::FindByFile(OpenFile* file) {
    for (int i = 0; i < MAX_INODES; ++i) {
        if (inodes[i] != nullptr && inodes[i]->getFilePtr() == file) {
            return i;
        }
    }
    return -1;
}

OpenFile* InodeTable::GetFile(const inode_t inode) const {
    if (IsOpen(inode)) {
        return &inodes[inode]->getFile();
    }
    return nullptr;
}

int InodeTable::GetRefCount(const inode_t inode) const {
    if (IsOpen(inode)) {
        return inodes[inode]->getRefCount();
    }
    return -1;
}

int InodeTable::GetSector(const inode_t inode) const {
    if (IsOpen(inode)) {
        return inodes[inode]->getSector();
    }
    return -1;
}

void InodeTable::Print() const {
    printf("Inode Table:\n");
    for (int i = 0; i < MAX_INODES; ++i) {
        if (inodes[i]) {
            printf("Inode %d: Sector %d, RefCount %d\n", i, inodes[i]->getSector(), inodes[i]->getRefCount());
        } else {
            printf("Inode %d: Free\n", i);
        }
    }
}
