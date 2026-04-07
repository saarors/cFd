#include "cmd_whoami.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  include <pwd.h>
#  include <sys/types.h>
#endif

int cmd_whoami(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; (void)argc; (void)argv;

#ifdef _WIN32
    char name[256];
    DWORD sz = sizeof(name);
    if (GetUserNameA(name, &sz)) {
        printf("%s\n", name);
        return 0;
    }
    /* Fallback */
    const char *u = getenv("USERNAME");
    if (u) { printf("%s\n", u); return 0; }
    fprintf(stderr, "whoami: cannot determine user name\n");
    return 1;
#else
    /* Try getlogin first */
    char *login = getlogin();
    if (login) {
        printf("%s\n", login);
        return 0;
    }
    /* Fall back to getpwuid */
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        printf("%s\n", pw->pw_name);
        return 0;
    }
    /* Last resort: USER env */
    const char *u = getenv("USER");
    if (u) { printf("%s\n", u); return 0; }
    fprintf(stderr, "whoami: cannot determine user name\n");
    return 1;
#endif
}

const cfd_command_t builtin_whoami = {
    "whoami",
    "whoami",
    "Print the current user name",
    "system",
    cmd_whoami,
    0, 0
};
