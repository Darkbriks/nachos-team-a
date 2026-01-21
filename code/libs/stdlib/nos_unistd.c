#include "syscall.h"
#include "nos_string.h"
#include "nos_unistd.h"

bool close_file(int __fd){
    return Close(__fd) == 0;
}

ssize_t read (int __fd, void *__buf, size_t __nbytes){
    int size = Read(__buf, __nbytes, __fd);
    ((char *)__buf)[size] = 0;
    return size;
}

ssize_t write (int __fd, const void *__buf, size_t __n){ 
    return Write((char *)__buf, __n, __fd);
}

int open(char* name, int flags){
    if (flags && O_CREATE != 0){
        if (! create(name) ){
            return -1;
        }
    }
    return Open(name, strlen(name));
}

bool create(char *name){
    return Create(name, strlen(name)) == 0;
}
