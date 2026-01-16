#ifndef CODE_PATHNAVIGATOR_H
#define CODE_PATHNAVIGATOR_H

#include "fileconst.h"

class FileSystem;
class OpenFile;

constexpr int MaxPathComponent = FileNameMaxLen + 1;
constexpr int MaxPathDepth = 64;

class PathNavigator {
public:
    explicit PathNavigator(FileSystem* fs, const char* path, bool autoRestore = true);
    ~PathNavigator();

    PathNavigator(const PathNavigator&) = delete;
    PathNavigator& operator=(const PathNavigator&) = delete;
    PathNavigator(PathNavigator&&) = delete;
    PathNavigator& operator=(PathNavigator&&) = delete;

    bool isValid() const;
    const char* getLastComponent() const;
    int getDepth() const;
    const char* getComponent(int index) const;

    void disableAutoRestore();
    bool restoreToBackup() const;

private:
    FileSystem* _fs;
    int _backupDirectorySector;

    char _components[MaxPathDepth][MaxPathComponent];
    int _depth;
    bool _valid;
    bool _autoRestore;

    int parsePath(const char* path);
    bool navigateToParent();
};

#endif
