#include "cmd_read.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/select.h>
#endif

int cmd_read(cfd_session_t *sess, int argc, char **argv) {
    const char *prompt   = NULL;
    int         nchars   = 0;   /* -n: read at most N chars */
    int         timeout  = -1;  /* -t: timeout in seconds, -1 = none */
    int         var_start = 1;  /* index of first variable name */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            prompt = argv[++i];
            var_start = i + 1;
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            nchars = atoi(argv[++i]);
            var_start = i + 1;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
            var_start = i + 1;
        } else {
            /* First non-option is start of variable names */
            var_start = i;
            break;
        }
    }

    /* Print prompt */
    if (prompt) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

#ifndef _WIN32
    /* Timeout support on Unix */
    if (timeout >= 0) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv;
        tv.tv_sec  = timeout;
        tv.tv_usec = 0;
        int sel = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (sel <= 0) return 1; /* timeout or error */
    }
#endif

    char buf[4096];
    if (nchars > 0 && nchars < (int)sizeof(buf)) {
        /* Read exactly nchars */
        int i;
        for (i = 0; i < nchars; i++) {
            int c = fgetc(stdin);
            if (c == EOF || c == '\n') break;
            buf[i] = (char)c;
        }
        buf[i] = '\0';
    } else {
        if (!fgets(buf, sizeof(buf), stdin)) return 1;
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    }

    int var_count = argc - var_start;
    if (var_count <= 0) return 0;

    if (var_count == 1) {
        /* Assign entire line to single variable */
        cfd_session_set_var(sess, argv[var_start], buf);
        return 0;
    }

    /* Multiple variables: first N-1 get one word each, last gets the rest */
    char *p = buf;
    for (int v = var_start; v < argc; v++) {
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;

        if (*p == '\0') {
            cfd_session_set_var(sess, argv[v], "");
            continue;
        }

        if (v == argc - 1) {
            /* Last variable gets the rest (trim trailing space) */
            size_t len = strlen(p);
            while (len > 0 && (p[len-1] == ' ' || p[len-1] == '\t')) len--;
            char tmp[4096];
            strncpy(tmp, p, len);
            tmp[len] = '\0';
            cfd_session_set_var(sess, argv[v], tmp);
        } else {
            /* Find end of word */
            char *start = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            char tmp[4096];
            size_t wlen = (size_t)(p - start);
            if (wlen >= sizeof(tmp)) wlen = sizeof(tmp) - 1;
            strncpy(tmp, start, wlen);
            tmp[wlen] = '\0';
            cfd_session_set_var(sess, argv[v], tmp);
        }
    }

    return 0;
}

const cfd_command_t builtin_read = {
    "read",
    "read [-p prompt] [-n chars] [-t timeout] <varname...>",
    "Read a line from stdin and assign to variables",
    "system",
    cmd_read,
    0, -1
};
