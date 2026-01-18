#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"

int main(){
    char buffer[200];
    int fd = Open("a");
    PutInt(Read(buffer, 200, fd));
    printf("on lit %s car fd = %d \n", buffer, fd);
    for (int i = 0; i < 10; i++){
        buffer[i] = 'z';
    }
    buffer[10] = 0;
    Write(buffer, 10, fd);
    Close(fd);
    fd = Open("a");
    Read(buffer, 200, fd);
    printf("on lit %s \n", buffer);
}
