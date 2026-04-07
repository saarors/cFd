#include "cmd_wc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
static void count_stream(FILE *f, long *lines, long *words, long *bytes) {
    *lines=*words=*bytes=0;
    int c, in_word=0;
    while ((c=fgetc(f))!=EOF) {
        (*bytes)++;
        if(c=='\n') (*lines)++;
        if(isspace(c)) in_word=0;
        else if(!in_word) { in_word=1; (*words)++; }
    }
}
int cmd_wc(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    bool show_l=false,show_w=false,show_c=false;
    int fi=1;
    for(int i=1;i<argc;i++) {
        if(argv[i][0]=='-') {
            for(char *p=argv[i]+1;*p;p++) {
                if(*p=='l')show_l=true;
                else if(*p=='w')show_w=true;
                else if(*p=='c')show_c=true;
            }
            fi=i+1;
        } else {fi=i;break;}
    }
    if (!show_l&&!show_w&&!show_c) show_l=show_w=show_c=true;
    long tl=0,tw=0,tc=0;
    if (fi>=argc) {
        long l,w,c; count_stream(stdin,&l,&w,&c);
        if(show_l)printf("%7ld ",l); if(show_w)printf("%7ld ",w); if(show_c)printf("%7ld",c);
        printf("\n");
        return 0;
    }
    for(int i=fi;i<argc;i++) {
        FILE *f=fopen(argv[i],"r");
        if(!f){perror(argv[i]);continue;}
        long l,w,c; count_stream(f,&l,&w,&c); fclose(f);
        tl+=l;tw+=w;tc+=c;
        if(show_l)printf("%7ld ",l); if(show_w)printf("%7ld ",w); if(show_c)printf("%7ld ",c);
        printf("%s\n",argv[i]);
    }
    if (argc-fi>1) {
        if(show_l)printf("%7ld ",tl);if(show_w)printf("%7ld ",tw);if(show_c)printf("%7ld ",tc);
        printf("total\n");
    }
    return 0;
}
const cfd_command_t builtin_wc = {
    "wc","wc [-lwc] [file...]","Word, line, byte count","text",cmd_wc,0,-1
};
