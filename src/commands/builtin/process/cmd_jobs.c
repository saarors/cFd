#include "cmd_jobs.h"
#include <stdio.h>
int cmd_jobs(cfd_session_t *sess, int argc, char **argv) {
    (void)argc;(void)argv;
    if (sess->job_count == 0) { printf("No background jobs.\n"); return 0; }
    for (int i=0;i<sess->job_count;i++){
        printf("[%d] %5d  %-10s  %s\n", i+1, sess->jobs[i].pid,
               sess->jobs[i].status==0?"Running":"Stopped",
               sess->jobs[i].cmd);
    }
    return 0;
}
const cfd_command_t builtin_jobs = {
    "jobs","jobs","List background jobs","process",cmd_jobs,0,0
};
