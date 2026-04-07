#include "cmd_ln.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

int cmd_ln(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool symbolic = false;
    bool force    = false;
    const char *target = NULL;
    const char *link_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) symbolic = true;
        else if (strcmp(argv[i], "-f") == 0) force = true;
        else if (!target)    target    = argv[i];
        else if (!link_name) link_name = argv[i];
    }

    if (!target || !link_name) {
        fprintf(stderr, "Usage: ln [-s] [-f] <target> <link>\n");
        return 1;
    }

#ifdef _WIN32
    if (force) {
        DeleteFileA(link_name);
        RemoveDirectoryA(link_name);
    }

    if (symbolic) {
        /* Determine if target is a directory */
        DWORD attr = GetFileAttributesA(target);
        DWORD flags = (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
                      ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
        flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

        if (!CreateSymbolicLinkA(link_name, target, flags)) {
            fprintf(stderr, "ln: cannot create symbolic link '%s': error %lu\n",
                    link_name, GetLastError());
            fprintf(stderr, "ln: Note: symbolic links on Windows may require elevated privileges or Developer Mode\n");
            return 1;
        }
    } else {
        if (!CreateHardLinkA(link_name, target, NULL)) {
            fprintf(stderr, "ln: cannot create hard link '%s': error %lu\n",
                    link_name, GetLastError());
            return 1;
        }
    }
#else
    if (force) {
        unlink(link_name);
    }

    int rc;
    if (symbolic)
        rc = symlink(target, link_name);
    else
        rc = link(target, link_name);

    if (rc != 0) {
        perror("ln");
        return 1;
    }
#endif

    return 0;
}

const cfd_command_t builtin_ln = {
    "ln",
    "ln [-s] [-f] <target> <link>",
    "Create links between files",
    "fs",
    cmd_ln,
    2, -1
};
