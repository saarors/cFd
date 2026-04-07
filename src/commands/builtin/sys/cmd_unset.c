#include "cmd_unset.h"
#include "../../../scripting/variable.h"
int cmd_unset(cfd_session_t *sess, int argc, char **argv) {
    for (int i=1;i<argc;i++) cfd_var_unset(sess->vars, argv[i]);
    return 0;
}
const cfd_command_t builtin_unset = {
    "unset","unset <name...>","Unset shell variables","system",cmd_unset,1,-1
};
