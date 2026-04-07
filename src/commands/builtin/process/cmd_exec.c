#include "cmd_exec.h"
#include "../../../platform/platform.h"
#include "../../../utils/path.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
int cmd_exec(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    if (argc < 2) { fprintf(stderr,"exec: missing command\n"); return 1; }
    char *path_env = cfd_platform_getenv("PATH");
    int n; char **dirs = cfd_path_split_dirs(path_env, &n);
    cfd_free(path_env);
    char *prog = NULL;
    if (cfd_path_is_absolute(argv[1])) { prog = cfd_strdup(argv[1]); }
    else {
        for (int i=0;i<n&&!prog;i++){
            char *full = cfd_path_join(dirs[i], argv[1]);
#ifdef _WIN32
            char *fullexe = cfd_sprintf("%s.exe", full);
            if (cfd_path_exists(fullexe)){cfd_free(full);prog=fullexe;}
            else {cfd_free(fullexe);}
#endif
            if (!prog && cfd_path_exists(full)) prog = full; else cfd_free(full);
        }
    }
    cfd_strfreev(dirs);
    if (!prog) { fprintf(stderr,"exec: %s: not found\n",argv[1]); return 127; }
    int ret = cfd_platform_run_program(prog, argv+1, NULL);
    cfd_free(prog);
    return ret;
}
const cfd_command_t builtin_exec = {
    "exec","exec <prog> [args...]","Execute a program","process",cmd_exec,1,-1
};
