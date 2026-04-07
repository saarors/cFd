#include "cmd_date.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
int cmd_date(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    const char *fmt = "%Y-%m-%d %H:%M:%S %Z";
    if (argc > 1 && argv[1][0]=='+') fmt = argv[1]+1;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[256];
    strftime(buf, sizeof(buf), fmt, t);
    puts(buf);
    return 0;
}
const cfd_command_t builtin_date = {
    "date","date [+format]","Print current date and time","system",cmd_date,0,1
};
