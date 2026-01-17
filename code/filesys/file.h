#ifndef FILE_H
#define FILE_H

#include "copyright.h"
#include "fileconst.h"
#include "fileindirection.h"
#include "filetype.h"

class FileEntry;
class OpenFile;
class FirstIndirection;

class File {
public:
    explicit File();
    ~File(); 

    void FetchFrom(sector_t sector) const;
    void WriteBack(sector_t sector) const;


    void Print() const; 

    FirstIndirection indirect;
    int x;
private:
};


#endif // FILE_H
