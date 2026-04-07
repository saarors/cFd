#include "cmd_cut.h"
#include "../../../utils/str_utils.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cmd_cut(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    char delim = '\t'; int field = -1;
    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i],"-d")==0&&i+1<argc) delim=argv[++i][0];
        else if(strcmp(argv[i],"-f")==0&&i+1<argc) field=atoi(argv[++i]);
    }
    if (field < 1) { fprintf(stderr,"cut: -f required\n"); return 1; }

    char line[65536];
    while(fgets(line,sizeof(line),stdin)){
        size_t l=strlen(line);
        while(l>0&&(line[l-1]=='\n'||line[l-1]=='\r'))line[--l]='\0';
        int n; char tmp[2]={delim,'\0'};
        char **parts=cfd_strsplit(line,tmp,&n);
        if(field<=n) printf("%s\n",parts[field-1]);
        else printf("\n");
        cfd_strfreev(parts);
    }
    return 0;
}
const cfd_command_t builtin_cut = {
    "cut","cut -f N [-d delim]","Cut fields from lines","text",cmd_cut,0,-1
};
