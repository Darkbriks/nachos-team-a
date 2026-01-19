#include "filesysshell.h"

#include <iostream>
#include <vector>

#include "system.h"
#include "filesys.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#define COMMAND_0(name, description, action) \
    commands.push_back(new Command(name, description, {}, action));

#define COMMAND_1(name, description, arg1, desc1, action) \
    commands.push_back(new Command(name, description, {std::make_shared<Arg>(arg1, desc1)}, action));

#define COMMAND_2(name, description, arg1, desc1, arg2, desc2, action) \
    commands.push_back(new Command(name, description, {std::make_shared<Arg>(arg1, desc1), std::make_shared<Arg>(arg2, desc2)}, action));

void FileSysShell::registerCommands(){
    COMMAND_0("help", "Display this help message", [this](const std::vector<std::string>&){
        this->printHelp();
    });

    COMMAND_0("quit", "Exit the filesystem shell", [](const std::vector<std::string>&){
        std::cout << "Exiting filesystem shell." << std::endl << "Bye!" << std::endl;
        exit(0);
    });

    COMMAND_0("exit", "Alias for quit", [](const std::vector<std::string>&){
        std::cout << "Exiting filesystem shell." << std::endl << "Bye!" << std::endl;
        exit(0);
    });

    COMMAND_0("q", "Alias for quit", [](const std::vector<std::string>&){
        std::cout << "Exiting filesystem shell." << std::endl << "Bye!" << std::endl;
        exit(0);
    });

    COMMAND_0("clear", "Clear the console screen", [](const std::vector<std::string>&){
        std::cout << "\033[2J\033[1;1H";
    });

    COMMAND_0("ls", "List files in the current directory", [this](const std::vector<std::string>&){
        this->fileSystem->List();
    });

    COMMAND_0("tree", "Display the directory tree", [this](const std::vector<std::string>&){
        this->fileSystem->Tree();
    });

    COMMAND_0("debug", "Print the entire filesystem structure", [this](const std::vector<std::string>&){
        this->fileSystem->Print_FS();
    });

    COMMAND_0("inodes", "Print the entire inodes table structure", [this](const std::vector<std::string>&){
        this->fileSystem->DisplayInodes();
    });

    COMMAND_0("test", "Run filesystem performance test", [](const std::vector<std::string>&){
        PerformanceTest();
    });

    COMMAND_1("mkdir", "Create a new directory",
              "dirname", "Name of the directory to create",
              [this](const std::vector<std::string>& args){
        this->fileSystem->Create(args[0].c_str(), DirectoryFileSize, DIRECTORY_T);
    });

    COMMAND_1("cd", "Change the current directory",
              "dirname", "Name of the directory to change to",
              [this](const std::vector<std::string>& args){
        this->fileSystem->Change_Directory(args[0].c_str());
    });

    COMMAND_1("rm", "Remove a file or directory",
              "name", "Name of the file or directory to remove",
              [this](const std::vector<std::string>& args){
        this->fileSystem->Remove(args[0].c_str());
    });

    COMMAND_1("cat", "Display the contents of a file",
              "filename", "Name of the file to display",
              [this](const std::vector<std::string>& args){
        this->fileSystem->ReadAllFile(args[0].c_str());
    });

    COMMAND_2("cp", "Copy a file from UNIX to Nachos",
              "source", "Source file path in UNIX",
              "dest", "Destination file path in Nachos",
              [](const std::vector<std::string>& args){
        Copy(args[0].c_str(), args[1].c_str());
    });
}

FileSysShell::FileSysShell(FileSystem* fs) : fileSystem(fs) {}

FileSysShell::~FileSysShell() {
    for (const auto cmd : commands) {
        delete cmd;
    }
}

void FileSysShell::printHelp() const {
    std::cout << "Available commands:\n";
    for (const auto cmd : commands) {
        std::cout << "  - " << std::left << std::setw(10) << cmd->getName() << ": " << cmd->getDescription() << "\n";
    }
}

void FileSysShell::run() {
    char input[MAX_COMMAND_SIZE];

    while (true) {
        const char *cwd = fileSystem->GetWorkingPath();
        std::cout << "[kernel@nachos " << cwd << " (sector: " << fileSystem->GetWorkingSector() << ")]$ ";
        delete[] cwd;

        std::cin.getline(input, MAX_COMMAND_SIZE);

        if (std::cin.eof()) {
            std::cout << "\nExiting filesystem shell." << std::endl << "Bye!" << std::endl;
            break;
        }

        std::string inputStr(input);
        std::istringstream iss(inputStr);
        std::string cmdName;
        iss >> cmdName;

        if (cmdName.empty()) { continue; }

        auto it = std::find_if(commands.begin(), commands.end(),
                               [&cmdName](const Command* cmd) { return cmd->getName() == cmdName; });

        if (it != commands.end()) {
            Command* cmd = *it;
            std::vector<std::string> argValues;
            std::string arg;
            while (iss >> arg) {
                argValues.push_back(arg);
            }
            cmd->execute(argValues);
        } else {
            std::cout << "Unknown command: " << cmdName << "\n";
            printHelp();
        }
    }
}

void Command::execute(const std::vector<std::string>& argValues)const {
    if (argValues.size() != args.size()) {
        std::cout << "Usage: " << name;
        for (const auto& arg : args) {
            std::cout << " <" << arg->getName() << ">";
        }
        std::cout << "\n";

        for (const auto & arg : args) {
            std::cout << "  " << arg->getName() << ": " << arg->getDescription() << "\n";
        }

        return;
    }
    action(argValues);
}
