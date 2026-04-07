#include "cmd_sort.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool g_reverse = false;
static bool g_numeric = false;

static int sort_cmp(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    int r;
    if (g_numeric) r = atoi(sa) - atoi(sb);
    else r = strcmp(sa, sb);
    return g_reverse ? -r : r;
}

int cmd_sort(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    g_reverse=false; g_numeric=false;
    int fi=1;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"-r")==0){g_reverse=true;fi=i+1;}
        else if(strcmp(argv[i],"-n")==0){g_numeric=true;fi=i+1;}
        else {fi=i;break;}
    }

    FILE *f = (fi < argc) ? fopen(argv[fi],"r") : stdin;
    if (!f && fi < argc) { perror(argv[fi]); return 1; }

    char **lines = cfd_malloc(65536 * sizeof(char*));
    int n = 0; char buf[65536];
    while (fgets(buf, sizeof(buf), f)) {
        size_t l = strlen(buf);
        while(l>0&&(buf[l-1]=='\n'||buf[l-1]=='\r'))buf[--l]='\0';
        lines[n++] = cfd_strdup(buf);
        if (n >= 65536) break;
    }
    if (f != stdin) fclose(f);

    qsort(lines, n, sizeof(char*), sort_cmp);
    for (int i=0;i<n;i++) { puts(lines[i]); cfd_free(lines[i]); }
    cfd_free(lines);
    return 0;
}
const cfd_command_t builtin_sort = {
    "sort","sort [-rn] [file]","Sort lines of text","text",cmd_sort,0,1
};
