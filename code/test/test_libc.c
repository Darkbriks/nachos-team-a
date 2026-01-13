#include "syscall.h"
#include "nos_stdlib.h"
#include "nos_string.h"

int main(){

    int x = 0;
    scanf_simple("%d", &x);
    PutInt(x);
    if (x != 1){
        printf_simple("Erreur\n");
    }
    char c = 'a';
    scanf_simple("%c", &c);
    printf_simple(&c);
    if (c != 'b'){
        printf_simple("Erreur\n");
    }
    char *s = "salut";
    scanf_simple("%s", s);
    if ( strcmp(s, "hey") != 0 ){
        printf_simple("Erreur\n");
    }
    printf_simple("Sucess\n");


}
