#include "my_stdlib.h"

void my_printf(char *buf){
    PutString(buf, my_strlen(buf));
}

void my_scanf(char *format, ...){}

unsigned int my_strlen(char *str){
    unsigned int result = 0;
    while(str[result] != '\0'){result++;}
    return result;
}

void print_error(const char *msg) {
    my_printf((char *) msg);
    my_printf(" (errno=");
    PutInt(GetLastError());
    my_printf(")\n");
}
