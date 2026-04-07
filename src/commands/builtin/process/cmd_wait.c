#include "cmd_wait.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/wait.h>
#  include <unistd.h>
#  include <errno.h>
#endif

int cmd_wait(cfd_session_t *sess, int argc, char **argv) {
#ifdef _WIN32
    (void)sess; (void)argc; (void)argv;
    /* Windows does not have background job management in this shell */
    printf("wait: background job management not supported on Windows\n");
    return 0;
#else
    if (argc >= 2) {
        /* Wait for specific PID */
        int ret = 0;
        for (int i = 1; i < argc; i++) {
            pid_t pid = (pid_t)atoi(argv[i]);
            if (pid <= 0) {
                fprintf(stderr, "wait: invalid pid: %s\n", argv[i]);
                ret = 1;
                continue;
            }
            int status = 0;
            pid_t r = waitpid(pid, &status, 0);
            if (r < 0) {
                if (errno == ECHILD)
                    fprintf(stderr, "wait: %d: no such child process\n", pid);
                else
                    perror("wait");
                ret = 1;
            } else {
                if (WIFEXITED(status)) {
                    int ec = WEXITSTATUS(status);
                    if (ec != 0) ret = ec;
                    printf("[%d] done (exit %d)\n", pid, ec);
                } else if (WIFSIGNALED(status)) {
                    printf("[%d] killed by signal %d\n", pid, WTERMSIG(status));
                    ret = 1;
                }
            }
        }

        /* Also update job table in session */
        if (sess) {
            for (int i = 1; i < argc; i++) {
                int pid = atoi(argv[i]);
                for (int j = 0; j < sess->job_count; j++) {
                    if (sess->jobs[j].pid == pid)
                        sess->jobs[j].status = 2; /* done */
                }
            }
        }
        return ret;
    }

    /* Wait for all background jobs */
    if (!sess || sess->job_count == 0) {
        /* Wait for any child */
        int status = 0;
        pid_t r;
        while ((r = waitpid(-1, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status))
                printf("[%d] done (exit %d)\n", r, WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                printf("[%d] killed by signal %d\n", r, WTERMSIG(status));
        }
        return 0;
    }

    /* Wait for jobs in session's job table */
    for (int j = 0; j < sess->job_count; j++) {
        if (sess->jobs[j].status == 0 || sess->jobs[j].status == 1) {
            int status = 0;
            pid_t r = waitpid(sess->jobs[j].pid, &status, 0);
            if (r > 0) {
                sess->jobs[j].status = 2;
                if (WIFEXITED(status))
                    printf("[%d] %s done (exit %d)\n",
                           sess->jobs[j].pid, sess->jobs[j].cmd,
                           WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    printf("[%d] %s killed by signal %d\n",
                           sess->jobs[j].pid, sess->jobs[j].cmd,
                           WTERMSIG(status));
            }
        }
    }
    return 0;
#endif
}

const cfd_command_t builtin_wait = {
    "wait",
    "wait [pid...]",
    "Wait for background jobs to complete",
    "process",
    cmd_wait,
    0, -1
};
