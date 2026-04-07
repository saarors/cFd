#include "cmd_echo.h"
#include <stdio.h>
#include <string.h>
int cmd_echo(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; bool newline = true; int start = 1;
    if (argc > 1 && strcmp(argv[1],"-n")==0) { newline=false; start=2; }
    for (int i = start; i < argc; i++) {
        if (i > start) putchar(' ');
        fputs(argv[i], stdout);
    }
    if (newline) putchar('\n');
    fflush(stdout);
    return 0;
}
const cfd_command_t builtin_echo = {
    "echo","echo [-n] [args...]","Print arguments","text",cmd_echo,0,-1
};
