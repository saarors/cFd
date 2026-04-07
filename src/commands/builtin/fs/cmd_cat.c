#include "cmd_cat.h"
#include <stdio.h>
#include <string.h>
static int cat_file(const char *path, bool number) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return 1; }
    char line[4096]; int n = 1;
    while (fgets(line, sizeof(line), f)) {
        if (number) printf("%6d  %s", n++, line);
        else fputs(line, stdout);
    }
    fclose(f); return 0;
}
int cmd_cat(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; bool number = false; int ret = 0;
    int start = 1;
    if (argc > 1 && strcmp(argv[1],"-n")==0) { number=true; start=2; }
    if (start >= argc) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), stdin)) fputs(buf, stdout);
        return 0;
    }
    for (int i = start; i < argc; i++) ret |= cat_file(argv[i], number);
    return ret;
}
const cfd_command_t builtin_cat = {
    "cat","cat [-n] [file...]","Concatenate and print files","filesystem",cmd_cat,0,-1
};
