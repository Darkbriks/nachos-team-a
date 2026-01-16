#include "pathnavigator.h"
#include "filesys.h"
#include "system.h"

PathNavigator::PathNavigator(FileSystem* fs, const char* path, const bool autoRestore)
    : _fs(fs), _backupDirectoryFile(nullptr), _depth(0), _valid(false), _autoRestore(autoRestore)
{
    ASSERT(fs != nullptr);
    ASSERT(path != nullptr);

    for (int i = 0; i < MaxPathDepth; i++) { _components[i][0] = '\0'; }

    _backupDirectoryFile = _fs->GetCurrentDirectory();

    _depth = parsePath(path);
    if (_depth <= 0) {
        DEBUG('p', "PathNavigator: Failed to parse path '%s'\n", path);
        return;
    }

    DEBUG('p', "PathNavigator: Parsed %d components from '%s'\n", _depth, path);
    for (int i = 0; i < _depth; i++) {
        DEBUG('p', "  [%d] = '%s'\n", i, _components[i]);
    }

    _valid = navigateToParent();

    if (!_valid) {
        DEBUG('p', "PathNavigator: Navigation failed, restoring directory\n");
        _fs->SetCurrentDirectory(_backupDirectoryFile);
    }
}

PathNavigator::~PathNavigator() {
    if (_autoRestore && _backupDirectoryFile != nullptr) {
        DEBUG('p', "PathNavigator: Restoring directory on destruction\n");
        _fs->SetCurrentDirectory(_backupDirectoryFile);
    }
}

bool PathNavigator::isValid() const {
    return _valid;
}

const char* PathNavigator::getLastComponent() const {
    if (_depth <= 0) {
        return nullptr;
    }
    return _components[_depth - 1];
}

int PathNavigator::getDepth() const {
    return _depth;
}

const char* PathNavigator::getComponent(const int index) const {
    if (index < 0 || index >= _depth) {
        return nullptr;
    }
    return _components[index];
}

void PathNavigator::disableAutoRestore() {
    _autoRestore = false;
}

bool PathNavigator::restoreToBackup() const {
    if (_backupDirectoryFile == nullptr) {
        return false;
    }
    _fs->SetCurrentDirectory(_backupDirectoryFile);
    return true;
}

int PathNavigator::parsePath(const char* path) {
    bool isEscaped = false;
    int posInComponent = 0;
    int componentIndex = 0;

    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == '\\') {
            isEscaped = !isEscaped;
        } else if (path[i] == '/') {
            if (!isEscaped && posInComponent > 0) {
                _components[componentIndex][posInComponent] = '\0';
                componentIndex++;
                posInComponent = 0;
            }
            continue;
        }
        if (!isEscaped) {
            _components[componentIndex][posInComponent] = path[i];
            posInComponent++;
            isEscaped = false;
        }
    }

    if (posInComponent > 0) {
        _components[componentIndex][posInComponent] = '\0';
        componentIndex++;
    }

    return componentIndex;
}

bool PathNavigator::navigateToParent() {
    for (int i = 0; i < _depth - 1; i++) {
        DEBUG('p', "PathNavigator: Changing directory to '%s'\n", _components[i]);
        if (!_fs->Change_Directory(_components[i])) {
            DEBUG('p', "PathNavigator: Failed to change directory to '%s'\n", _components[i]);
            return false;
        }
    }
    return true;
}