#include "cmd_export.h"
#include "../../../platform/platform.h"
#include "../../../scripting/variable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmd_export(cfd_session_t *sess, int argc, char **argv) {
    if (argc < 2) {
        /* List all environment variables */
        char **envp = cfd_platform_environ();
        if (envp) {
            for (int i = 0; envp[i]; i++)
                printf("export %s\n", envp[i]);
        }
        return 0;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        char *eq  = strchr(arg, '=');

        if (eq) {
            /* NAME=value */
            char name[256];
            size_t nlen = (size_t)(eq - arg);
            if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
            strncpy(name, arg, nlen);
            name[nlen] = '\0';

            const char *value = eq + 1;

            /* Set in session vars */
            cfd_session_set_var(sess, name, value);

            /* Export to environment */
            if (cfd_platform_setenv(name, value) != 0) {
                fprintf(stderr, "export: failed to set %s\n", name);
                ret = 1;
            }
        } else {
            /* NAME - export existing variable */
            const char *value = cfd_session_get_var(sess, arg);
            if (!value) {
                /* Try environment */
                value = cfd_platform_getenv(arg);
            }
            if (value) {
                if (cfd_platform_setenv(arg, value) != 0) {
                    fprintf(stderr, "export: failed to export %s\n", arg);
                    ret = 1;
                }
            } else {
                /* Export as empty string */
                cfd_platform_setenv(arg, "");
            }
        }
    }
    return ret;
}

const cfd_command_t builtin_export = {
    "export",
    "export [name[=value]...]",
    "Set and export environment variables",
    "system",
    cmd_export,
    0, -1
};
