#include "cmd_kill.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#ifdef _WIN32
#  include <windows.h>
#endif
int cmd_kill(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    if (argc < 2) { fprintf(stderr,"kill: missing pid\n"); return 1; }
    int sig = 15; int fi = 1;
    if (argc > 1 && argv[1][0]=='-') { sig=atoi(argv[1]+1); fi=2; }
    int ret = 0;
    for (int i=fi;i<argc;i++){
        int pid = atoi(argv[i]);
#ifdef _WIN32
        HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,(DWORD)pid);
        if(!h||!TerminateProcess(h,1)){fprintf(stderr,"kill: %d: no such process\n",pid);ret=1;}
        if(h)CloseHandle(h);
#else
        if (kill(pid, sig) != 0) { perror(argv[i]); ret=1; }
#endif
    }
    return ret;
}
const cfd_command_t builtin_kill = {
    "kill","kill [-sig] <pid...>","Send signal to process","process",cmd_kill,1,-1
};
