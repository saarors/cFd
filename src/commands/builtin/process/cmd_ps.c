#include "cmd_ps.h"
#include "../../../platform/platform.h"
#include <stdio.h>
int cmd_ps(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;(void)argc;(void)argv;
    printf("  PID  CMD\n");
    printf("%5d  cFd\n", cfd_platform_get_pid());
    /* List background jobs from session */
    for (int i = 0; i < sess->job_count; i++) {
        printf("%5d  %s  [%s]\n",
               sess->jobs[i].pid, sess->jobs[i].cmd,
               sess->jobs[i].status == 0 ? "running" : "stopped");
    }
    return 0;
}
const cfd_command_t builtin_ps = {
    "ps","ps","List running processes/jobs","process",cmd_ps,0,0
};
