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
        if (i >= 0) {
            pid = ForkExec(buffer);
            PutInt(pid);
            if (pid < 0){
                print_error("Pas de process crée\n");
                continue;;
            }
            ForkJoin(pid, &exitCode);
            printf_simple("voici le code de retour du process\n");
            PutInt(exitCode);
        }
    }
}

