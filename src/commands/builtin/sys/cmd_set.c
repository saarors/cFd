#include "cmd_set.h"
#include "../../../scripting/variable.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>
int cmd_set(cfd_session_t *sess, int argc, char **argv) {
    if (argc < 2) {
        /* List all vars */
        int count=0;
        char **keys = cfd_var_list(sess->vars, &count);
        for (int i=0;i<count;i++){
            const char *v=cfd_var_get(sess->vars,keys[i]);
            printf("%s=%s\n",keys[i],v?v:"");
            cfd_free(keys[i]);
        }
        cfd_free(keys);
        return 0;
    }
    /* set NAME VALUE or set NAME=VALUE */
    char *eq = strchr(argv[1], '=');
    if (eq) {
        *eq='\0';
        cfd_session_set_var(sess, argv[1], eq+1);
        *eq='=';
    } else if (argc >= 3) {
        cfd_session_set_var(sess, argv[1], argv[2]);
    } else {
        cfd_session_set_var(sess, argv[1], "");
    }
    return 0;
}
const cfd_command_t builtin_set = {
    "set","set [name [value]]","Set or list shell variables","system",cmd_set,0,-1
};
