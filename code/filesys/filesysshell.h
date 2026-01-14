#ifndef CODE_FILESYSSHELL_H
#define CODE_FILESYSSHELL_H

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// c++ std librarie is used here because this code is only for the shell interface
// and is not really part of the Nachos kernel.
//

extern void Copy(const char *unixFile, const char *nachosFile);
extern void Print(const char *file);
extern void PerformanceTest(void);

class FileSystem;

class Arg {
private:
    std::string name;
    std::string description;

public:
    Arg(const std::string& n, const std::string& desc)
        : name(std::move(n)), description(std::move(desc)) {}

    ~Arg() = default;

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getDescription() const { return description; }
};

class Command {
private:
    std::string name;
    std::string description;
    std::vector<std::shared_ptr<Arg>> args;
    std::function<void(const std::vector<std::string>&)> action;
    
public:
    Command(std::string n, std::string desc,
            const std::vector<std::shared_ptr<Arg>>& a,
            std::function<void(const std::vector<std::string>&)> ac)
        : name(std::move(n)), description(std::move(desc)), args(a), action(std::move(ac)) {}
    
    ~Command() = default;
    
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getDescription() const { return description; }
    [[nodiscard]] const std::vector<std::shared_ptr<Arg>>& getArgs() const { return args; }
    
    void execute(const std::vector<std::string>& argValues)const;
};

class FileSysShell {
private:
    static constexpr int MAX_COMMAND_SIZE = 256;
    std::vector<Command*> commands;

    FileSystem* fileSystem;
    
public:
    explicit FileSysShell(FileSystem* fs);
    ~FileSysShell();

    void printHelp() const;
    void run();
    void registerCommands();
};

#endif
