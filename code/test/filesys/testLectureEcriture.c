#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_unistd.h"

int main(){
    char buffer[200];
    int fd = open("a", 0);
    PutInt(read(fd, buffer, 200));
    printf("on lit %s car fd = %d \n", buffer, fd);
    for (int i = 0; i < 10; i++){
        buffer[i] = 'z';
    }
    buffer[10] = 0;

    close(fd);
    fd = open("a", 0);
    write(fd, buffer, 10);
    close(fd);
    fd = open("a", 0);
    read(fd, buffer, 200);
    close(fd);
    printf("on lit %s \n", buffer);
}
