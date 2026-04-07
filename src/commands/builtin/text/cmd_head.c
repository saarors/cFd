#include "cmd_head.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void head_stream(FILE *f, int n) {
    char line[65536]; int i=0;
    while(i++<n&&fgets(line,sizeof(line),f)) fputs(line,stdout);
}
int cmd_head(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; int n=10; int fi=1;
    if(argc>1&&argv[1][0]=='-'&&argv[1][1]!='\0'){n=atoi(argv[1]+1);fi=2;}
    if(fi>=argc){head_stream(stdin,n);return 0;}
    for(int i=fi;i<argc;i++){
        if(argc-fi>1)printf("==> %s <==\n",argv[i]);
        FILE *f=fopen(argv[i],"r");if(!f){perror(argv[i]);continue;}
        head_stream(f,n);fclose(f);
    }
    return 0;
}
const cfd_command_t builtin_head = {
    "head","head [-n] [file...]","Print first lines of file","text",cmd_head,0,-1
};
