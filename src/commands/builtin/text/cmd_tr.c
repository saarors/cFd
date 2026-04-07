#include "cmd_tr.h"
#include <stdio.h>
#include <string.h>
int cmd_tr(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    if (argc < 3) { fprintf(stderr,"tr: missing operands\n"); return 1; }
    const char *src = argv[1], *dst = argv[2];
    int slen=(int)strlen(src), dlen=(int)strlen(dst);
    int c;
    while((c=fgetc(stdin))!=EOF){
        bool found=false;
        for(int i=0;i<slen;i++){
            if(c==(unsigned char)src[i]){
                int di=(i<dlen)?i:dlen-1;
                putchar(dst[di]);
                found=true; break;
            }
        }
        if(!found) putchar(c);
    }
    return 0;
}
const cfd_command_t builtin_tr = {
    "tr","tr <set1> <set2>","Translate characters","text",cmd_tr,2,2
};
