#include "cmd_tail.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
static void tail_stream(FILE *f, int n) {
    char **ring = cfd_calloc(n, sizeof(char*));
    int idx=0, total=0; char line[65536];
    while(fgets(line,sizeof(line),f)){
        cfd_free(ring[idx%n]);
        ring[idx%n]=cfd_strdup(line);
        idx++; total++;
    }
    int start=(total>=n)?(idx-n):0;
    for(int i=start;i<idx;i++){fputs(ring[i%n],stdout);}
    for(int i=0;i<n;i++)cfd_free(ring[i]);
    cfd_free(ring);
}
int cmd_tail(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; int n=10; int fi=1;
    if(argc>1&&argv[1][0]=='-'&&argv[1][1]!='\0'){n=atoi(argv[1]+1);fi=2;}
    if(fi>=argc){tail_stream(stdin,n);return 0;}
    for(int i=fi;i<argc;i++){
        if(argc-fi>1)printf("==> %s <==\n",argv[i]);
        FILE *f=fopen(argv[i],"r");if(!f){perror(argv[i]);continue;}
        tail_stream(f,n);fclose(f);
    }
    return 0;
}
const cfd_command_t builtin_tail = {
    "tail","tail [-n] [file...]","Print last lines of file","text",cmd_tail,0,-1
};
