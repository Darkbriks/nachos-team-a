#include "nos_errno.h"
#include "nos_stdio.h"
#include "nos_string.h"
#include "syscall.h"

int main() {
    const char prompt[] = "user@nos-shell > ";
    const char exitCmd[] = "exit";
    char buffer[255];
    int exitCode = -1;

    while (1) {
        printf("%s", prompt); fflush(stdout);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("\nsession terminée\n");
            break;
        }

        const size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') { buffer[len - 1] = '\0'; }

        if (strcmp(buffer, exitCmd) == 0) {
            printf("session terminée\n");
            break;
        }

        const int pid = ForkExec(buffer, sizeof(buffer));
        if (pid < 0){
            printf("shell : échec de lancement du programme '%s' (errno=%d)\n", buffer, errno);
            continue;
        }
        ForkJoin(pid, &exitCode);
        if (exitCode < 0) {
            printf("shell : le programme s'est terminé de manière anormale (code=%d)\n", exitCode);
        }
    }
}
