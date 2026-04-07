#include "cmd_pwd.h"
#include <stdio.h>
int cmd_pwd(cfd_session_t *sess, int argc, char **argv) {
    (void)argc; (void)argv;
    puts(sess->cwd ? sess->cwd : ".");
    return 0;
}
const cfd_command_t builtin_pwd = {
    "pwd","pwd","Print working directory","filesystem",cmd_pwd,0,0
};
