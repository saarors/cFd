#include "cmd_grep.h"
#include "../../../ui/color.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int grep_stream(FILE *f, const char *pat, const char *fname,
                       bool invert, bool icase, bool linenum, bool count_only) {
    char line[65536]; int n=0, matches=0;
    while (fgets(line, sizeof(line), f)) {
        n++;
        char *hay = line;
        char tmp[65536];
        if (icase) {
            strncpy(tmp, line, sizeof(tmp)-1);
            for (char *p=tmp;*p;p++) if(*p>='A'&&*p<='Z') *p+=32;
            hay = tmp;
        }
        bool found = strstr(hay, pat) != NULL;
        if (invert) found = !found;
        if (found) {
            matches++;
            if (!count_only) {
                if (fname) printf("%s:", fname);
                if (linenum) printf("%d:", n);
                /* highlight match */
                if (!icase && cfd_color_enabled()) {
                    char *pos = line;
                    char *m;
                    while ((m = strstr(pos, pat)) != NULL) {
                        fwrite(pos, 1, m - pos, stdout);
                        printf("%s%s%s", FG_BRIGHT_RED, pat, COLOR_RESET);
                        pos = m + strlen(pat);
                    }
                    fputs(pos, stdout);
                } else {
                    fputs(line, stdout);
                }
            }
        }
    }
    if (count_only) printf("%d\n", matches);
    return matches > 0 ? 0 : 1;
}

int cmd_grep(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    bool invert=false, icase=false, linenum=false, count_only=false;
    const char *pat=NULL; int ret=1;
    int fi = 1;
    for (int i=1;i<argc;i++) {
        if(argv[i][0]=='-') {
            for (char *p=argv[i]+1;*p;p++) {
                if(*p=='v') invert=true;
                else if(*p=='i') icase=true;
                else if(*p=='n') linenum=true;
                else if(*p=='c') count_only=true;
            }
            fi=i+1;
        } else { fi=i; break; }
    }
    if (fi >= argc) { fprintf(stderr,"grep: missing pattern\n"); return 1; }
    pat = argv[fi++];
    char cpat[65536]; strncpy(cpat,pat,sizeof(cpat)-1);
    if (icase) for(char *p=cpat;*p;p++) if(*p>='A'&&*p<='Z') *p+=32;

    if (fi >= argc) {
        ret = grep_stream(stdin, cpat, NULL, invert, icase, linenum, count_only);
    } else {
        for (int i=fi;i<argc;i++) {
            FILE *f=fopen(argv[i],"r");
            if(!f){perror(argv[i]);continue;}
            ret &= grep_stream(f, cpat, argc-fi>1?argv[i]:NULL,
                               invert,icase,linenum,count_only);
            fclose(f);
        }
    }
    return ret;
}
const cfd_command_t builtin_grep = {
    "grep","grep [-vinc] <pattern> [file...]","Search text patterns","text",cmd_grep,1,-1
};
