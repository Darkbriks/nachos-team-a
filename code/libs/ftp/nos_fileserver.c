#include "nos_fileserver.h"
#include "nos_unistd.h"
/* Parse command from received data */
int serverParseCommand(char *data, char *cmd, char *arg1, int *arg2) {
    int i = 0, j = 0;

    /* Extract command */
    while (data[i] != '\0' && data[i] != ' ' && j < 15) {
        cmd[j++] = data[i++];
    }
    cmd[j] = '\0';

    /* Skip space */
    if (data[i] == ' ') i++;

    /* Extract first argument (filename) */
    j = 0;
    while (data[i] != '\0' && data[i] != ' ' && j < MAX_FILENAME - 1) {
        arg1[j++] = data[i++];
    }
    arg1[j] = '\0';

    /* Extract second argument (size) if present */
    *arg2 = 0;
    if (data[i] == ' ') {
        i++;
        while (data[i] >= '0' && data[i] <= '9') {
            *arg2 = (*arg2) * 10 + (data[i] - '0');
            i++;
        }
    }

    return 0;
}
