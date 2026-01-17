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
  File();
  ~File();

  void FetchFrom(sector_t sector) const;
  void WriteBack(sector_t sector) const;

  void Print();
  sector_t getFirstIndirection() { return indirect; }
  void setFirstIndirection(sector_t i) { indirect = i; }
  void setRedirect2(sector_t s, int i) { second_indercetion[i] = s; }
  sector_t getRedirect2(int i) { return second_indercetion[i]; }

  sector_t indirect;
  // Second Indirection
  sector_t second_indercetion[MAX_INDIRECT_LEVEL_TWO];
private:
};

static_assert(sizeof(File) == SectorSize);

#endif // FILE_H
