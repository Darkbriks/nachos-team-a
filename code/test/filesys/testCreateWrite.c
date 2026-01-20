#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "nos_unistd.h"
#include "nos_string.h"
#include "test_utilities.h"

void do_Test(){
    TEST_START("Create and write");
    char buffer[] = "coucou la mif, en directu du fileSystem\n";
    int fd = open("z", 0);
    ASSERT_NEQ(0, fd, "Open Sucess but file doesn't exist\n\n");
    fd = open("z", O_CREATE);
    ASSERT_EQ(0, fd, "Opend with create fail\n");
    ASSERT_EQ(40, write(fd, buffer, strlen(buffer)), "Don't write enought data\n");
    ASSERT_EQ(0, close(fd), "Can't close\n");
    fd = open("z", 0);
    buffer[0] = 0;
    ASSERT_EQ(40, read(fd, buffer, 41), "Don't read enought data\n");
    ASSERT_EQ(0, close(fd), "Can't close\n");
    ASSERT_STREQ(buffer, "coucou la mif, en directu du fileSystem\n", "Don't read good data\n");
    TEST_PASS();

}

int main(){
    do_Test();
    return 1;
}

