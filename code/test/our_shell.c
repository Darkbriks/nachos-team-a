#include "nos_stdlib.h"
#include "syscall.h"

int main() {
    char prompt[] = "our_shell >  ";
    char buffer[255];
    int i;
    int pid;
    int exitCode = -1;
    while (1) {
        printf_simple(prompt);
        i = scanf_simple("%s", buffer);
        if (i < 0) {
            printf_simple("\nsession terminée\n");
            break;
        } else{
            pid = ForkExec(buffer);
            if (pid < 0){

                print_error("Pas de process crée");
                printf_simple(" pour mot entré = "); 
                printf_simple(buffer); 
                PutInt(buffer[0]); 
                printf_simple("\n");
                continue;;
            }
            ForkJoin(pid, &exitCode);
            if (exitCode < 0){
                printf_simple("shell voit une mauvaise terminaison avec code :");
                PutInt(exitCode);
                printf_simple(" \n");
            }
        }     
    }
}
