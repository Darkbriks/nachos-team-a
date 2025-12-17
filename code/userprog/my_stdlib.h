#ifndef MY_STD_LIB_C
#define MY_STD_LIB_C

#include "syscall.h"


void my_printf(char *buf);
void my_scanf(char *format, ...);

unsigned int my_strlen(char *str);

void print_error(const char *msg); 


#endif //MY_STD_LIB_C
